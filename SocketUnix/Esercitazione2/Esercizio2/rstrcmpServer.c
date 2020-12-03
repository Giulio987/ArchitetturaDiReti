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
#define MAXLEN 2048

int main(int argc, char **argv){
    int sd, nread, err;
    struct addrinfo hints, *res;
    char frase1[MAXLEN], frase2[MAXLEN];

    if(argc != 2){
        fprintf(stderr, "Inserire il numero di porta\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    err = getaddrinfo(NULL, argv[1], &hints, &res);
    if(err < 0){
        fprintf(stderr, "Errore getaddrinfo\n");
        exit(1);
    }
    if((sd = socket(res->ai_family,res->ai_socktype, res->ai_protocol)) < 0){
        perror("Crerazione socket\n");
        exit(2);
    }
    if(bind(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("BIND\n");
        exit(3);
    }
    freeaddrinfo(res);
    if(listen(sd, SOMAXCONN) < 0){
        perror("Listen\n");
        exit(3);
    }
    while(1){
        int ns;
        struct sockaddr_storage client_address;
        socklen_t fromlen;

        fromlen = sizeof(client_address);

        ns = accept(sd,(struct sockaddr *)&client_address, &fromlen);
        if (ns < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("Errore in accept");
            exit(EXIT_FAILURE);
        }
        //close(sd);
        memset(frase1, 0, sizeof(frase1));
        memset(frase2, 0, sizeof(frase2));
        if((nread = read(ns, frase1, sizeof(frase1)-1)) < 0){
            perror("Read frase 1\n");
            close(ns);
            exit(4);
        }
        if(write(ns, "ACK1", 4) < 0){
                perror("Errore invio risposta\n");
                close(ns);
                exit(5);
            }
        if((nread = read(ns, frase2, sizeof(frase2)-1)) < 0){
            perror("Read frase 1\n");
            exit(4);
        }
        if(strcmp(frase1, frase2) != 0){
            printf("NO\n");
            if(write(ns, "NO", 2) < 0){
                perror("Errore invio risposta\n");
                exit(5);
            }
        }else{
            printf("Si\n");
            if(write(ns, "SI", 2) < 0){
                perror("Errore invio risposta\n");
                close(ns);
                exit(5);
            }
        }
        close(ns);
    }
    return 0;
}