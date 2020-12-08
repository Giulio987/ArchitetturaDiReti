#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
    int err, nread;
    struct addrinfo hints, *res, *ptr;
    int sd;
    rxb_t rxb;
    char response[MAX_REQUEST_SIZE];
    size_t response_len;
    /* Controllo argomenti */
    if (argc < 2)
    {
        printf("Uso: rgrep hostname porta stringa nomefile...\n");
        exit(EXIT_FAILURE);
    }

    /* Costruzione dell'indirizzo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* Risoluzione dell'host */
    if ((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        /*se socket fallisce salto direttamente alla prossima iterazione*/
        if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
        {
            fprintf(stderr, "creazione socket fallita\n");
            continue;
        }
        /*se connect funziona esco dal ciclo*/
        if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
        {
            break;
        }
        close(sd);
    }

    /* Verifica sul risultato restituito da getaddrinfo */
    if (ptr == NULL)
    {
        fprintf(stderr, "Errore risoluzione nome: nessun indirizzo corrispondente trovato\n");
        exit(EXIT_FAILURE);
    }

    /* Liberiamo la memoria allocata da getaddrinfo() */
    freeaddrinfo(res);

    /* Invio nome del file al server per la verifica*/

    if (write_all(sd, argv[4], strlen(argv[4])) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    /* 
    * Ricezione risultato 
    * Inizializzo buffer di ricezione 
    */
    rxb_init(&rxb, MAX_REQUEST_SIZE);

    memset(response, 0, sizeof(response));
    response_len = sizeof(response) - 1;

    //FUNZIONE che termina solo quando trova /n ma qui non
    //lo trova CAZZO!!! perche non c'era
    if (rxb_readline(&rxb, sd, response, &response_len) < 0)
    {
        rxb_destroy(&rxb);
        fprintf(stderr, "File non trovato, connessione chiusa\n");
        exit(EXIT_FAILURE);
    }

    //comunico al server la stringa da ricercare
    if (write_all(sd, argv[3], strlen(argv[3])) < 0)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    /*shutdown(sd, SHUT_WR);

    fflush(stdout);*/
    //recupero Informazioni server
    //anche qui non va bene xread perche 
    //ho degli /n
    memset(response, 0, sizeof(response));
    if ((nread = read(sd, response, sizeof(response) - 1)) < 0)
    {
        perror("read\n");
        exit(EXIT_FAILURE);
    }
    /* Stampo riga letta da Server */
    puts(response);

    /* chiudo socket */
    close(sd);

    return 0;
}
