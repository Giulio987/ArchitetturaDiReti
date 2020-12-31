#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "rxb.h"
#include "utils.h"
#include <limits.h>

/* La massima dimensione di una richiesta è di 64KiB */
#define MAX_REQUEST_SIZE (1024)

/* Gestore del segnale SIGCHLD */
void handler(int signo)
{
    int status;

    (void)signo; /* per evitare warning */

    /* eseguo wait non bloccanti finché ho dei figli terminati */
    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int main(int argc, char **argv)
{
    int sd, err, on;
    char *end_request = "--END--\n", *ack = "ack\n";
    struct addrinfo hints, *res;
    struct sigaction sa;

    if (argc != 2)
    {
        fprintf(stderr, "Sintassi: %s porta\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Preparo direttive getaddrinfo */
    memset(&hints, 0, sizeof(hints));
    /* Usa AF_INET per forzare solo IPv4, AF_INET6 per forzare solo IPv6 */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* Uso getaddrinfo per preparare le strutture dati da usare con socket e bind */
    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    /* Creo la socket */
    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("Errore in socket");
        exit(EXIT_FAILURE);
    }

    /* Disabilo attesa uscita fase TIME_WAIT prima di creazioe socket */
    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    /* Metto in ascolto la socket sulla porta desiderata */
    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Errore in bind");
        exit(EXIT_FAILURE);
    }

    /* A questo punto, posso liberare la memoria allocata da getaddrinfo */
    freeaddrinfo(res);

    /* Trasforma in socket passiva d'ascolto */
    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {

        int ns, pid;

        /* Mi metto in attesa di richieste di connessione */
        puts("Server in ascolto");
        ns = accept(sd, NULL, NULL);
        if (ns < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        /* Creo un processo figlio per gestire la richiesta */
        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        { /* FIGLIO  per ogni richiesta*/
            rxb_t rxb;
            char anno[MAX_REQUEST_SIZE], N[MAX_REQUEST_SIZE], tipologia[MAX_REQUEST_SIZE];
            int pid2, p1p2[2], p2p0[2];
            size_t request_len;
            char percorso[(MAX_REQUEST_SIZE*2) + 25], end[4096];

            /* Chiudo la socket passiva */
            close(sd);
            
            /* Avvio ciclo gestione richieste */
            for (;;)
            {   int  sum = 0;
                /* Inizializzo buffer di ricezione */
                rxb_init(&rxb, MAX_REQUEST_SIZE);

                /*ricevo l'anno di interesse*/
                memset(anno, 0, sizeof(anno));
                request_len = sizeof(anno) - 1;

                /* Leggo richiesta da Client */
                if (rxb_readline(&rxb, ns, anno, &request_len) < 0)
                {
                    rxb_destroy(&rxb);
                    break;
                }
                    
                //mandol'ack
                if (write_all(ns, ack, strlen(ack)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                //ricevo il numero di spese di interesse
                memset(N, 0, sizeof(N));
                request_len = sizeof(N) - 1;
                if (rxb_readline(&rxb, ns, N, &request_len) < 0)
                {
                    rxb_destroy(&rxb);
                    break;
                }
                //mandol'ack
                if (write_all(ns, ack, strlen(ack)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                //ricevo la tipologia di spese di interesse
                memset(tipologia, 0, sizeof(tipologia));
                request_len = sizeof(tipologia) - 1;
                if (rxb_readline(&rxb, ns, tipologia, &request_len) < 0)
                {
                    rxb_destroy(&rxb);
                    break;
                }
                //creo la stringa per il percorso
                snprintf(percorso, sizeof(percorso), "/var/local/expenses/%s/%s.txt", anno,tipologia);
                // la grep per tipologia e anno non servono visto che le tipologie all'interno del file
                // sono le stesse del nome del file 
                
                if (pipe(p1p2) < 0)
                {
                    perror("pipe");
                    exit(5);
                }
                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0) //nipote 1
                {
                    /* Chiudo la socket attiva */
                    close(ns);
                    close(p1p2[0]);
                    close(1);
                    if (dup(p1p2[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[1]);

                    execlp("sort", "sort", "-n","-r", percorso, (char *)NULL);
                    perror("execlp");
                    exit(6);
                }
                
                //figlio
                if (pipe(p2p0) < 0)
                {
                    perror("pipe");
                    exit(5);
                }

                close(p1p2[1]);
                
                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0) //nipote 2
                {
                    /* Chiudo la socket attiva */
                    close(ns);
                    close(p2p0[0]);
                    close(0);
                    if (dup(p1p2[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    /* Chiudo la socket attiva */
                    close(p1p2[0]);

                    close(1);
                    if (dup(p2p0[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p0[1]);
                    execlp("head", "head", "-n", N, (char *)NULL);
                    perror("execFiglio");
                    exit(8);
                }
                //FIGLIO
                //chiudo pipe aperte
                close(p1p2[0]);
                close(p2p0[1]);
                //CLICLO PER LA GESTIONE STRINGHE
                for (;;)
                {
                    char response[MAX_REQUEST_SIZE], response2[MAX_REQUEST_SIZE +2];
                    memset(response, 0, sizeof(response));
                    request_len = sizeof(response) - 1;  
                    if (rxb_readline(&rxb, p2p0[0], response, &request_len) < 0)
                    {
                        rxb_destroy(&rxb);
                        break;
                    }
                    snprintf(response2,sizeof(response2), "%s\n", response);
                    //SCRIVO SULLA SCOKET IL RISULTATO
                    if (write_all(ns, response2, strlen(response2)) < 0)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    
                    char *token = strtok(response, ",");
                    long ret;
                    char *endptr;
                    ret = strtol(token, &endptr, 10);

                    if (ret == 0 && errno == EINVAL)
                    {
                        break;
                    }
                    if (errno == ERANGE)
                    {
                        if (ret == LONG_MIN)
                        {
                            fprintf(stderr, "Underflow\n");
                            break;
                        }
                        else
                        { // ret == LONG_MAX
                            fprintf(stderr, "Overflow\n");
                            break;
                        }
                    }
                    //fprintf(stdout, "ret= %lu\n",ret);
                    sum += ret;
                }
                close(p2p0[0]);
                //creo la stringa finale
                snprintf(end, sizeof(end), "Il totale per le spese di interesse è: %d\n", sum);
                if (write_all(ns, end, strlen(end)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                //invio terminatore
                if (write_all(ns, end_request, strlen(end_request)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
            close(ns);
            exit(EXIT_SUCCESS);
        }
        /* padreChiudo la socket attiva */
        close(ns);
    }

    /* Chiudo la socket passiva (just in case) */
    close(sd);

    return 0;
}