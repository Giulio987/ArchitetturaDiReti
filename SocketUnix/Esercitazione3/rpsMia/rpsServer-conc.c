#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

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
    int sd, err, on;
    struct addrinfo hints, *res;
    typedef int pipe_t[2];
    struct sigaction sa;

    /* Controllo argomenti */
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s porta\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sigemptyset(&sa.sa_mask);
    /* uso SA_RESTART per evitare di dover controllare esplicitamente se
         * accept è stata interrotta da un segnale e in tal caso rilanciarla
         * (si veda il paragrafo 21.5 del testo M. Kerrisk, "The Linux
         * Programming Interface") */
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    /* Preparo direttive getaddrinfo */
    memset(&hints, 0, sizeof(hints));
    /* Usa AF_INET per forzare solo IPv4, AF_INET6 per forzare solo IPv6 */
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0)
    {
        fprintf(stderr, "Errore recupero informazioni\n");
        exit(1);
    }
    if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        perror("Creazione Socket\n");
        exit(2);
    }
    on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("Sockopt\n");
        exit(3);
    }
    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Bind\n");
        exit(4);
    }
    freeaddrinfo(res);
    if (listen(sd, SOMAXCONN) < 0)
    {
        perror("Listen\n");
        exit(5);
    }

    for (;;)
    {
        char request[2048], response[4096];
        int ns, nread, pid, status;
        pipe_t p1ps;
        ns = accept(sd, NULL, NULL);

        if (ns < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("accept");
            exit(EXIT_FAILURE);
        }

        memset(request, 0, sizeof(request));
        if ((nread = read(ns, request, sizeof(request) - 1)) < 0)
        {
            perror("read");
            close(ns);
            continue;
        }

        if (pipe(p1ps) < 0)
        {
            perror("Pipe");
            exit(4);
        }
        if ((pid = fork()) < 0)
        {
            perror("fork");
        }
        if (pid == 0)
        { //figlio
            //close(sd);
            close(p1ps[0]);
            close(1);
            dup(p1ps[1]);
            if (!strcmp(request, "request"))
            {
                execlp("ps", "ps", (char *)0);
            }
            else
            {
                execlp("ps", "ps", request, (char *)0);
            }
            perror("execlp");
            exit(7);
        }
        close(p1ps[1]);
        memset(response, 0, sizeof(response));
        while ((nread = read(p1ps[0], response, sizeof(response))) < 0)
        {
            perror("readPipe");
            close(ns);
            continue;
        }
        close(p1ps[0]);
        wait(&status);
        if (write(ns, response, strlen(response)) < 0)
        {
            perror("write");
            close(ns);
            continue;
        }

        /* Chiudo la socket attiva */
        close(ns);
    }
    close(sd);
    return 0;
}