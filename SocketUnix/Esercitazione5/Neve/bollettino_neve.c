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
    char regione[MAX_REQUEST_SIZE], nLocal[MAX_REQUEST_SIZE], ack[10];
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


    for (;;)
    {   
        //Inizio il buffer di ricezione a ogni nuvo ciclo di richieste
        rxb_init(&rxb, MAX_REQUEST_SIZE);

        puts("Inserisci il nome della regione: ");
        if (fgets(regione, sizeof(regione), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }
        if(strcmp(regione, "fine\n") == 0){
            break;
        }
        puts("Inserisci Il numero di loaclità: ");
        if (fgets(nLocal, sizeof(nLocal), stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if(strcmp(nLocal, "fine\n") == 0){
            break;
        }
        /* Invio richiesta al Server compreso il /n*/
        if (write_all(sd, regione, strlen(regione)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
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
        //Invio il numero di località di interesse
        if (write_all(sd, nLocal, strlen(nLocal)) < 0)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        /* È consigliabile effettuare il flushing del buffer di printf
        *  prima di iniziare a scrivere sullo standard output con write */
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