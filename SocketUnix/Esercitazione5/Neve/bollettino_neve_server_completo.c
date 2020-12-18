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
#define MAX_REQUEST_SIZE (64 * 1024)

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
    char *end_request = "--END CONNECTION--\n", *ack = "ack\n";
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
            rxb_t rxb1, rxb2, rxb3;
            char request[MAX_REQUEST_SIZE], request2[MAX_REQUEST_SIZE], media[4096];
            int pid2, status, p1p2[2], p2p0[2];
            size_t request_len, request_len2;
            char regione[MAX_REQUEST_SIZE + 15];

            /* Disabilito gestore SIGCHLD */
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) == -1)
            {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            /* Chiudo la socket passiva */
            close(sd);

            /* Inizializzo buffer di ricezione */
            rxb_init(&rxb1, MAX_REQUEST_SIZE);
            rxb_init(&rxb2, MAX_REQUEST_SIZE);
            
            /* Avvio ciclo gestione categorie */
            for (;;)
            {   int  sum = 0;
                //DEVO REINIZIALIZZARE IL BUFFER PRIMA DI
                //UN NUOVO CICLO DI READ
                rxb_init(&rxb3, MAX_REQUEST_SIZE);
                /*RICEVO il nome della regione*/
                memset(request, 0, sizeof(request));
                request_len = sizeof(request) - 1;

                /* Leggo richiesta da Client */
                if (rxb_readline(&rxb1, ns, request, &request_len) < 0)
                {
                    rxb_destroy(&rxb1);
                    break;
                }
                    
                //creo la stringa
                snprintf(regione, sizeof(regione), "/var/local/%s.txt", request);

                //mandol'ack
                if (write_all(ns, ack, strlen(ack)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                //ricevo il numero di località
                memset(request2, 0, sizeof(request2));
                request_len2 = sizeof(request2) - 1;
                if (rxb_readline(&rxb2, ns, request2, &request_len2) < 0)
                {
                    rxb_destroy(&rxb2);
                    break;
                }
                
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
                    close(ns);
                    close(p1p2[0]);
                    close(1);
                    if (dup(p1p2[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[1]);

                    execlp("sort", "sort", "-n", "-r", regione, (char *)NULL);
                    perror("execlp");
                    exit(6);
                }
                //aspetto nipote 1
                wait(&status);
                close(p1p2[1]);
                //figlio
                if (pipe(p2p0) < 0)
                {
                    perror("pipe2");
                    exit(5);
                }
                
                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0) //nipote 2
                {
                    close(ns);
                    close(0);
                    if (dup(p1p2[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p1p2[0]);

                    close(1);
                    if (dup(p2p0[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    /* Chiudo la socket attiva */
                    close(p2p0[1]);
                    execlp("head", "head", "-n", request2, (char *)NULL);
                    perror("execFiglio");
                    exit(8);
                }
                //FIGLIO
                //Aspetto nipoti terminare
                wait(&status);
                //chiudo pipe aperte
                close(p1p2[0]);
                close(p2p0[1]);
                //CLICLO PER LA GESTIONE STRINGHE
                for (;;)
                {
                    char response[MAX_REQUEST_SIZE], response2[MAX_REQUEST_SIZE +2];
                    size_t response_len;
                    memset(response, 0, sizeof(response));
                    response_len = sizeof(response) - 1;  
                    if (rxb_readline(&rxb3, p2p0[0], response, &response_len) < 0)
                    {
                        rxb_destroy(&rxb3);
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
                            // underflow
                            break;
                        }
                        else
                        { // ret == LONG_MAX
                            // overflow
                            break;
                        }
                    }
                    //fprintf(stdout, "ret= %lu\n",ret);
                    sum += ret;
                }
                //CONVERTO IN NUMERO ANCHE IL NUMERO DI LOCALITA RICHIESTE
                long ret;
                char *endptr;
                ret = strtol(request2, &endptr, 10);

                if (ret == 0 && errno == EINVAL)
                {
                    fprintf(stderr, "Numero ricevuto non valido\n");
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
                        // overflow
                        fprintf(stderr, "Overflow\n");
                        break;
                    }
                }
                sum = sum / ret;

                close(p2p0[0]);
                snprintf(media, sizeof(media), "Media di neve per le localita inserite: %d\n", sum);
                if (write_all(ns, media, strlen(media)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
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