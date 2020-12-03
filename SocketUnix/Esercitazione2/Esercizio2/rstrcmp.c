#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAXLEN 2048

int main(int argc, char **argv){
    int sd, err, nread;
    struct addrinfo hints, *res, *ptr;
    char buf[MAXLEN], ACK[MAXLEN];
    if(argc != 5){
        fprintf(stderr,"Uso corretto: rstrcmp hostname porta stringa1 stringa2\n");
        exit(1);
    }
    //inizializzo flags
    memset(&hints, 0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //provo a recuperare le informazioni
    err = getaddrinfo(argv[1], argv[2],&hints, &res);
    if(err != 0){
        perror("Problema ocnnessione\n");
        exit(2);
    }
    //provo una connessione con fallback
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next){
        sd = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
        if(sd < 0){
            continue;
        }
        if(connect(sd,ptr->ai_addr,ptr->ai_addrlen) == 0){
            break;
        }
        close(sd);
    }
    if(ptr == NULL){
        fprintf(stderr,"Non è stato possibile stabilire una connessione\n");
        exit(3);
    }
    freeaddrinfo(res);
    //Scrivo prima frase;
    if(write(sd, argv[3], strlen(argv[3])) < 0){
        perror("Probema write 1\n");
        exit(4);
    }
    if (read(sd, ACK, strlen(ACK)) < 0) {
        perror("ACK");
        exit(EXIT_FAILURE);
    }
    printf("ACK\n");
    if(write(sd, argv[4], strlen(argv[4])) < 0){
        perror("Problema write 2\n");
        exit(4);
    }

    memset(buf, 0, sizeof(buf));
    if((nread = read(sd, buf,sizeof(buf)-1)) < 0){
        perror("Problema read\n");
        exit(5);
    }  
    fprintf(stdout, "%s\n", buf);
    close(sd);
    return 0;
}