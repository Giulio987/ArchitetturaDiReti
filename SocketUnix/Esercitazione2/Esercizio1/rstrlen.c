#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define MAXDIM 2048

int main(int argc, char **argv){
    struct addrinfo hints, *res, *ptr;
    int err, sd;
    long ret;
    char *endptr;
    char buf[MAXDIM], resp[MAXDIM];

    if(argc != 3){
        fprintf(stderr, "Uso corretto: rstrlen hostname porta");
        exit(1);
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(argv[1],argv[2],&hints, &res);
    if (err != 0) {
        fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    //fallback
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next){
        sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sd < 0) continue;
        if (connect(sd,ptr->ai_addr,ptr->ai_addrlen) == 0){
            break;
        }
        close(sd);
    }
    if (ptr == NULL) {
        fprintf(stderr, "Errore di connessione!\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    printf("Inserire la frase\n");
    scanf("%s", buf);
    while(strcmp(buf,"fine")){
        //assumo che la stringa venga scritta tutta e catturata
        //tutta dalla read dell'altra parte e viceversa
        if (write(sd, buf, strlen(buf)) < 0){
            perror("WRITE SOCKET\n");
            exit(2);
        }
        memset(resp, 0, sizeof(resp));
        if(read(sd, resp, sizeof(resp)-1) < 0 ){
            perror("ERRORE READ INPUT\n");
            exit(3);
        }
        if(atoi(resp) < 0){
            fprintf(stderr, "Formato non accettato\n");
            continue;
        }
        ret = strtol(resp, &endptr, 10);
        printf("La frase inserita contiene %lu caratteri\n", ret);
        printf("Inserire la frase\n ");
        scanf("%s", buf);
    }
    close(sd);
    return 0;
}