#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxb.h"
#include "utils.h"

/* La massima dimensione di una richiesta è di 64KiB */
#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    int sd, err;
    struct addrinfo hints, *ptr, *res;
    rxb_t rxb;
    char request[MAX_REQUEST_SIZE];
    char response[MAX_REQUEST_SIZE];
    size_t response_len;

    /* Controllo argomenti */
    if (argc != 3)
    {
        fprintf(stderr, "Sintassi: %s hostname porta\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Preparo direttive getaddrinfo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* Invoco getaddrinfo */
    err = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (err != 0)
    {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    /* Connessione con fallback */
    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        /* Se la socket fallisce, passo all'indirizzo successivo */
        if (sd < 0)
            continue;

        /* Se la connect va a buon fine, esco dal ciclo */
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
            break;

        /* Altrimenti, chiudo la socket e passo all'indirizzo
                 * successivo */
        close(sd);
    }

    /* Controllo che effettivamente il client sia connesso */
    if (ptr == NULL)
    {
        fprintf(stderr, "Errore di connessione!\n");
        exit(EXIT_FAILURE);
    }

    /* A questo punto, posso liberare la memoria allocata da getaddrinfo */
    freeaddrinfo(res);
    /* Inizializzo buffer di ricezione */
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    puts("Inserisci la categoria richiesta: ");
    if (fgets(request, sizeof(request), stdin) == NULL)
    {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    while (strcmp(request, "fine") != 0)
    {
        /* Invio richiesta al Server compreso il /n*/
        if (write_all(sd, request, strlen(request)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        //cosi assumo che quando il server chiude allora termino il ciclo
        for(;;)//while (strcmp(response, "--END CONNECTION--") != 0)
        {
            //FORSE ERA MEGLIO UNA SEMPLICE READ
            memset(response, 0, sizeof(response));
            response_len = sizeof(response) - 1;
            if (rxb_readline(&rxb, sd, response, &response_len) < 0)
            {
                rxb_destroy(&rxb);
                //fprintf(stderr, "Connessione chiusa dal server!\n");
                break;
            }
            puts(response);
        }
        /* Leggo stringa di richiesta */
        puts("Inserisci stringa di richiesta:");
        //fgets prende anche /n
        if (fgets(request, sizeof(request), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
    }
    close(sd);
    return 0;
}