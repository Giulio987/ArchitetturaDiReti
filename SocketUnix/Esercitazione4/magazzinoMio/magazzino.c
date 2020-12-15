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
    int sd, err, nread;
    struct addrinfo hints, *ptr, *res;
    rxb_t rxb;
    char *ackVer = "ack\n";
    char nomeVino[MAX_REQUEST_SIZE], annata[MAX_REQUEST_SIZE], ack[10];
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

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    puts("Inserisci il nome del vino richiesta: ");
    if (fgets(nomeVino, sizeof(nomeVino), stdin) == NULL)
    {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    puts("Inserisci l'annata: ");
    if (fgets(annata, sizeof(annata), stdin) == NULL)
    {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    while (strcmp(nomeVino, "fine\n") != 0)
    {
        /* Invio richiesta al Server compreso il /n*/
        if (write_all(sd, nomeVino, strlen(nomeVino)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        /* Inizializzo il buffer a zero e leggo sizeof(ack)-1
         * byte, così sono sicuro che il contenuto del buffer
         * sarà sempre null-terminated. In questo modo, posso
         * interpretarlo come una stringa C e passarlo
         * successivamente alla funzione strcmp. */
        memset(ack, 0, sizeof(ack));
        //per l'ack è meglio una semplice read
        if ((nread = read(sd, ack, sizeof(ack) - 1)) < 0)
        {
            perror("lettura ack");
            exit(EXIT_FAILURE);
        }
        //SEMPRE CONTROLLARE CHE L'ack SIA CORRETTO
        if (strcmp(ackVer, ack) != 0)
        {
            perror("strcmp ack");
            exit(EXIT_FAILURE);
        }
        if (write_all(sd, annata, strlen(annata)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        
        for (;;)
        {
            char response[MAX_REQUEST_SIZE];
            size_t response_len;

            memset(response, 0, sizeof(response));
            response_len = sizeof(response) - 1;

            if (rxb_readline(&rxb, sd, response, &response_len) < 0)
            {
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server!\n");
                exit(EXIT_FAILURE);
            }

            /* Stampo riga letta da Server */
            puts(response);

            /* Passo a nuova richiesta una volta terminato input Server */
            if (strcmp(response, "--END CONNECTION--") == 0)
            {
                break;
            }
        }
        puts("Inserisci il nome del vino richiesta: ");
        if (fgets(nomeVino, sizeof(nomeVino), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        puts("Inserisci l'annata: ");
        if (fgets(annata, sizeof(annata), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
    }
    close(sd);
    return 0;
}