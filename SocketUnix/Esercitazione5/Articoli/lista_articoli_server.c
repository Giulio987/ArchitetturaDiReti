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
int autorizza(const char *email_revisore, const char *password)
{
    return 1;
}

int main(int argc, char **argv)
{
    int sd, err, on;
    char *end_request = "--END CONNECTION--\n", *ack = "ack\n", *not_autorized = "CREDENZIALI NON CORRETTE\n";
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
        /*  grep cibo /var/local/conto_corrente.txt |sort -n -r*/

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
            char mail[MAX_REQUEST_SIZE], password[MAX_REQUEST_SIZE], rivista[MAX_REQUEST_SIZE], response[MAX_REQUEST_SIZE];
            int pid2, status, p1p2[2], p2p3[2], p3p4[2], nread;
            size_t request_len, request_len2, request_len3;

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
            rxb_init(&rxb3, MAX_REQUEST_SIZE);
            /* Avvio ciclo gestione categorie */
            for (;;)
            {
                /*RICEVO il nome del vino*/
                memset(mail, 0, sizeof(mail));
                request_len = sizeof(mail) - 1;

                /* Leggo richiesta da Client */
                if (rxb_readline(&rxb1, ns, mail, &request_len) < 0)
                {
                    rxb_destroy(&rxb1);
                    break;
                }
                //mandol'ack
                if (write_all(ns, ack, strlen(ack)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                //ricevo la password
                memset(password, 0, sizeof(password));
                request_len2 = sizeof(password) - 1;
                if (rxb_readline(&rxb2, ns, password, &request_len2) < 0)
                {
                    rxb_destroy(&rxb2);
                    break;
                }
                if (autorizza(mail, password) != 1)
                {
                    if (write_all(ns, not_autorized, strlen(not_autorized)) < 0)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);
                    exit(EXIT_SUCCESS);
                }
                else
                {
                    //mandol'ack
                    if (write_all(ns, ack, strlen(ack)) < 0)
                    {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                }
                memset(rivista, 0, sizeof(rivista));
                request_len3 = sizeof(rivista) - 1;
                if (rxb_readline(&rxb3, ns, rivista, &request_len3) < 0)
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
                    //trovo gli articoli con quella mail
                    execlp("grep", "grep", mail, "/var/local/revisione.txt", (char *)NULL);
                    perror("execlp");
                    exit(6);
                }
                //figlio
                if (pipe(p2p3) < 0)
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
                    close(p2p3[0]);
                    close(0);
                    if (dup(p1p2[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    /* Chiudo la socket attiva */
                    close(p1p2[0]);

                    close(1);
                    if (dup(p2p3[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    /* Chiudo la socket attiva */
                    close(ns);
                    close(p2p3[1]);
                    execlp("grep", "grep", rivista, (char *)NULL);
                    perror("execFiglio");
                    exit(8);
                }
                //FIGLIO
                close(p1p2[0]);
                close(p2p3[1]);
                if (pipe(p3p4) < 0)
                {
                    perror("pipe");
                    exit(5);
                }
                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0) //nipote 3
                {
                    close(p3p4[0]);
                    close(ns);
                    close(0);
                    if (dup(p2p3[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p2p3[0]);

                    close(1);
                    if (dup(p3p4[1]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p3p4[1]);

                    execlp("sort", "sort", "-r", (char *)NULL);
                    perror("execlp");
                    exit(6);
                }
                close(p2p3[0]);
                memset(response, 0, sizeof(response));
                if ((nread = read(p3p4[0], response, sizeof(response) - 1)) < 0)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                if (write_all(ns, response, strlen(response)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                if (write_all(p3p4[1], response, strlen(response)) < 0)
                {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                close(p3p4[1]);
                if ((pid2 = fork()) < 0)
                {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }
                else if (pid2 == 0) //nipote 4
                {
                    close(0);
                    if (dup(p3p4[0]) < 0)
                    {
                        perror("dup");
                        exit(EXIT_FAILURE);
                    }
                    close(p3p4[0]);

                    close(1);
                    if (dup(ns) < 0)
                    {
                        perror("dup socket");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);

                    execlp("wc", "wc", "-l", (char *)NULL);
                    perror("execlp");
                    exit(6);
                }
                //Aspetto nipoti terinare
                wait(&status);
                wait(&status);
                wait(&status);
                wait(&status);
                close(p3p4[0]);
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
