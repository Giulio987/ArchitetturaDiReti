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
#include "utils.h"


int main(int argc, char **argv)
{
        uint8_t buff[2048];
        int sd, err, nread;
        struct addrinfo hints, *res, *ptr;

	/* controllo argomenti */
        if (argc != 4) {
                fprintf(stderr, "Usage: client nomehost nomefilesorgente nomefiledestinazione\n");
                exit(EXIT_FAILURE);
        }

	/* inizializzo struttura hints con direttive per getaddrinfo */
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        err = getaddrinfo(argv[1], "50001", &hints, &res);
        if (err != 0) {
                fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }

        /* connessione con fallback */
        for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
                sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
                /* se la socket fallisce, passo all'indirizzo successivo */
                if (sd < 0) continue;

                /* se la connect va a buon fine, esco dal ciclo */
                if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
                        break;

                /* altrimenti, chiudo la socket e passo all'indirizzo
                 * successivo */
                close(sd);
        }

        /* controllo che effettivamente il client sia connesso */
        if (ptr == NULL) {
                fprintf(stderr, "Errore di connessione!\n");
                exit(EXIT_FAILURE);
        }

        /* a questo punto, posso liberare la memoria allocata da getaddrinfo */
        freeaddrinfo(res);

        /* scrivo nomefile */
        if (write_all(sd, argv[2], strlen(argv[2])) < 0) {
                perror("write");
                exit(EXIT_FAILURE);
        }

        /* leggo carattere S o N */
        nread = read(sd, buff, 1);
        if (nread < 0) {
                perror("read");
                exit(EXIT_FAILURE);
        }

        if (buff[0] == 'S') {
                /* il file esiste */
                int fd = open(argv[3], O_WRONLY|O_CREAT|O_EXCL, 
			      S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                if (fd < 0) {
                        perror("open");
                        exit(EXIT_FAILURE);
                }

                while ((nread = read(sd, buff, sizeof(buff))) > 0) {
                        if (write_all(fd, buff, nread) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
                }

                close(fd);

                printf("File ricevuto\n");
        } else {
                /* il file non esiste */
                printf("Il file richiesto non esiste, termino.\n");
        }

        close(sd);

        return 0;
}

