#define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rxb.h"
#include "utils.h"

#define MAX_REQUEST_SIZE (64 * 1024)

int main(int argc, char **argv)
{
	int err;
	struct addrinfo hints, *res, *ptr;
	char *host_remoto;
	char *servizio_remoto;
	int sd, i = 1;
	rxb_t rxb;

	/* Controllo argomenti */
	if (argc < 2)
	{
		printf("Uso: rps <server> <options>...\n");
		exit(EXIT_FAILURE);
	}

	/* Costruzione dell'indirizzo */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/* Risoluzione dell'host */
	host_remoto = argv[1];
	servizio_remoto = "50000";
	if ((err = getaddrinfo(host_remoto, servizio_remoto, &hints, &res)) != 0)
	{
		fprintf(stderr, "Errore risoluzione nome: %s\n", gai_strerror(err));
		exit(EXIT_FAILURE);
	}

	for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
	{
		/*se socket fallisce salto direttamente alla prossima iterazione*/
		if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
		{
			fprintf(stderr, "creazione socket fallita\n");
			continue;
		}
		/*se connect funziona esco dal ciclo*/
		if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
		{
			printf("connect riuscita al tentativo %d\n", i);
			break;
		}
		i++;
		close(sd);
	}

	/* Verifica sul risultato restituito da getaddrinfo */
	if (ptr == NULL)
	{
		fprintf(stderr, "Errore risoluzione nome: nessun indirizzo corrispondente trovato\n");
		exit(EXIT_FAILURE);
	}

	/* Liberiamo la memoria allocata da getaddrinfo() */
	freeaddrinfo(res);

	/* Inizializzo buffer di ricezione */
	rxb_init(&rxb, MAX_REQUEST_SIZE);

	for (;;)
	{
		char option[4096];

		/* Leggo stringa di richiesta */
		puts("Inserisci opzione per ps:");
		if (fgets(option, sizeof(option), stdin) == NULL)
		{
			perror("fgets");
			exit(EXIT_FAILURE);
		}

		/* Esco se l'utente digita . */
		if (strcmp(option, ".\n") == 0)
		{
			break;
		}

		/* Invio richiesta al Server 
                * write all è per dire di eseguire un ciclo di write fino 
                * a che non vengono scritti tutti i caratteri 
                * richiesti
                * Una write normale potrebbe scrivere solo la prima parte di option
                */
		if (write_all(sd, option, strlen(option)) < 0)
		{
			perror("write");
			exit(EXIT_FAILURE);
		}

		/* Leggo la risposta del server e la stampo a video */
		for (;;)
		{
			char response[MAX_REQUEST_SIZE];
			size_t response_len;

			/* Inizializzo il buffer response a zero e non uso l'ultimo
                         * byte, così sono sicuro che il contenuto del buffer sarà
                         * sempre null-terminated. In questo modo, posso interpretarlo
                         * come una stringa C. Questa è un'operazione che va svolta
                         * prima di leggere ogni nuova risposta. */
			memset(response, 0, sizeof(response));
			response_len = sizeof(response) - 1;

			/* Ricezione risultato 
                        * Effettuo un ciclo di letture finchè non ricevo il terminatore /n
                        * quando lo ricevo metto nel buffer response tutto il contenuto del 
                        * buf->buffer. se c'è rimasta della parte usata ma non copiata in destinazione
                        * (response) allora la copio all'inzio di buf->buffer
                        */
			if (rxb_readline(&rxb, sd, response, &response_len) < 0)
			{
				/* Se sono qui, è perché ho letto un EOF. Significa che
                                 * il Server ha chiuso la connessione, per cui dealloco
                                 * rxb (opzionale) ed esco. */
				rxb_destroy(&rxb);
				fprintf(stderr, "Connessione chiusa dal server!\n");
				exit(EXIT_FAILURE);
			}

			/* Stampo riga letta da Server */
			puts(response);

			/* Passo a nuova richiesta una volta terminato input Server */
			if (strcmp(response, "--- END REQUEST ---") == 0)
			{
				break;
			}
		}
	}

	/* chiudo socket */
	close(sd);

	return 0;
}
