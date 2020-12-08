#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include "utils.h"
#include "rxb.h"

#define MAX_REQUEST_SIZE (64 * 1024)

/* Gestore del segnale SIGCHLD */
void handler(int signo)
{
    int status;

    (void)signo;

    /* gestisco tutti i figli terminati */
    while (waitpid(-1, &status, WNOHANG) > 0)
        continue;
}

int main(int argc, char **argv)
{
    struct addrinfo hints, *res;
    int err, sd, ns, pid, on;
    struct sigaction sa;
    char *ack = "ACK\n";
    /* Controllo argomenti */
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s porta\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(hints));
    /* Usa AF_INET per forzare solo IPv4, AF_INET6 per forzare solo IPv6 */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0)
    {
        fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("Errore in socket");
        exit(EXIT_FAILURE);
    }

    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Errore in bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* Attendo i client... */
    for (;;)
    {
        printf("Server in ascolto...\n");

        if ((ns = accept(sd, NULL, NULL)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        /* Generazione di un figlio */
        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            int fd, nread;
            char nomeFile[MAX_REQUEST_SIZE], stringa[MAX_REQUEST_SIZE];
            /* figlio */
            close(sd); //chiudo socket passiva

            /* Inizializzo buffer di ricezione */

            memset(nomeFile, 0, sizeof(nomeFile));
        
            /* Leggo richiesta da Client */
            if ((nread = read(ns, nomeFile, sizeof(nomeFile)-1)) < 0)
            {
                close(ns); //quindi chiudo la socket attiva esco
                continue;
            }
            /* Per controllare se il file esiste, provo ad aprirlo */
            if ((fd = open(nomeFile, O_RDONLY)) < 0)
            {
                perror("errore apertura file\n");
                close(ns); //se il file non esiste chiudo la connessione
                continue;
            }
            /* Chiudo il file, che a questo punto sono certo che esista */
            close(fd);
            //Invio un ACK al client
            
            if (write_all(ns, ack, strlen(ack)) < 0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            //ricevo la srtringa
            memset(stringa, 0, sizeof(stringa));
            if ((nread = read(ns, stringa,sizeof(stringa)-1)) < 0)
            {
                close(ns); //quindi chiudo la socket attiva esco
                continue;
            }
            /* Ridireziono stdout e stderr */
            close(1);
            if (dup(ns) < 0)
            {
                perror("dup");
                exit(EXIT_FAILURE);
            }
            close(ns);
            //eseguo la grep

            execlp("grep", "grep", stringa, nomeFile, (char *)NULL);
            perror("exec grep\n");
            exit(EXIT_FAILURE);
        }
        /* PADRE */
        /* Chiudo la socket attiva */
        close(ns);
    }
    /* Chiudo la socket passiva (just in case) */
    close(sd);

    return 0;
}
