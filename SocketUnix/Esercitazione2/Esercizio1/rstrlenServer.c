#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define MAXLENGTH 500

int main(int argc, char **argv)
{
    int sd, err, ret, on;
    char buff[2048];
    struct addrinfo hints, *res;
    char response[MAXLENGTH];
    if (argc != 2)
    {
        fprintf(stderr, "Inserire i numero di porta come argomento\n");
        exit(1);
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
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
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Errore in bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    if (listen(sd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        int ns, nread;
        struct sockaddr_storage client_address;
        socklen_t fromlen;

        fromlen = sizeof(client_address);

        ns = accept(sd, (struct sockaddr *)&client_address, &fromlen);
        if (ns < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("Errore in accept");
            exit(EXIT_FAILURE);
        }
        
        memset(buff, 0, sizeof(buff));
        while ((nread = read(ns, buff, sizeof(buff) - 1)) > 0){
            ret = strnlen(buff,sizeof(buff));
            snprintf(response,sizeof(response),"%d\n", ret);
            if (write(ns, response, strlen(response)) < 0)
            {
                perror("Errore in write");
                close(ns);
            }
        }
        close(ns);
    }
    close(sd);
    return 0;
}