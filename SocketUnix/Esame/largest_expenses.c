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
    size_t response_len;
    char *ackVer = "ack";
    char anno[MAX_REQUEST_SIZE], N[MAX_REQUEST_SIZE], ack[10], tipologia[MAX_REQUEST_SIZE];
    /* Controllo argomenti */
    if (argc != 3)
    {
        fprintf(stderr, "Sintassi: %s server porta\n", argv[0]);
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


    for (;;)
    {   
        //Inizio il buffer di ricezione a ogni nuvo ciclo di richieste
        rxb_init(&rxb, MAX_REQUEST_SIZE);

        puts("Inserisci l'anno di interesse: ");
        if (fgets(anno, sizeof(anno), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if(strcmp(anno, "fine\n") == 0){
            break;
        }
        puts("Inserisci il numero di spese da visualizzare: ");
        if (fgets(N, sizeof(N), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if(strcmp(N, "fine\n") == 0){
            break;
        }
        puts("Inserisci la tipologia dispese di interesse: ");
        if (fgets(tipologia, sizeof(tipologia), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if(strcmp(tipologia, "fine\n") == 0){
            break;
        }
        /* Invio richieste al Server compreso il /n*/
        //Invio anno
        if (write_all(sd, anno, strlen(anno)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        //primo ack
        memset(ack, 0, sizeof(ack));
        response_len = sizeof(ack) - 1;
        if (rxb_readline(&rxb, sd, ack, &response_len) < 0)
        {
            rxb_destroy(&rxb);
            fprintf(stderr, "Connessione chiusa dal server!\n");
            break;
        }
        if (strcmp(ackVer, ack) != 0)
        {
            perror("Ack non corretto");
            exit(EXIT_FAILURE);
        }
        //Invio il numero di spese da visualizzare
        if (write_all(sd, N, strlen(N)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        //secondo ack
        memset(ack, 0, sizeof(ack));
        response_len = sizeof(ack) - 1;
        if (rxb_readline(&rxb, sd, ack, &response_len) < 0)
        {
            rxb_destroy(&rxb);
            fprintf(stderr, "Connessione chiusa dal server!\n");
            break;
        }
        //Invio la tipologia di interesse
        if (write_all(sd, tipologia, strlen(tipologia)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        //meglio effettuare il flush
        fflush(stdout);

        for (;;)
        {
            char response[MAX_REQUEST_SIZE];

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
            if (strcmp(response, "--END--") == 0)
            {
                break;
            }
        }
    }
    close(sd);
    return 0;
}