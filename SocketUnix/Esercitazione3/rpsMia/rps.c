#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAXDIM 2048

int main(int argc, char **argv)
{
	int sd, err, nread;
	struct addrinfo hints, *ptr, *res;
	char request[MAXDIM], response[2048];
	if ( (argc < 3) | (argc> 4))
	{
		fprintf(stderr, "Uso corretto: rps hostname porta <option>\n");
		exit(1);
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = AF_UNSPEC;
	hints.ai_family = AF_INET;

	err = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (err < 0)
	{
		fprintf(stderr, "Impossibile recuperare informazioni\n");
		exit(1);
	}
	for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
	{
		if ((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0)
		{
			continue;
		}
		if (connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0)
		{
			break;
		}
		close(sd);
	}
	if (ptr == NULL)
	{
		fprintf(stderr, "Impossibile stabilire una Connessione\n");
		exit(2);
	}
	freeaddrinfo(res);

	sprintf(request, "%s", "request");

	if (argc == 4)
	{
		snprintf(request, strlen(argv[3]), "%s", argv[3]);
	}
	/*uso strlen perchè:
	* argv[3] = ['a','u','x','/0']
	* strlen(argv[3]) = 3
	* sizeof(argv[3]) = 4 su macchina a 32 bit e 8 su macchina a 64 e se
	* avessi un argv[4] metteerebbe anche quello dentro
	*/
	if (write(sd, request, strlen(request)) < 0)
	{
		perror("Write 1\n");
		exit(3);
	}
	/* non posso assumere che vengano catturati con una sola read-> una read 1500 byte
	* di solito -> ammettendo 36000 byte dda trasferire(1 byte 1 carattere qua)
	* non posso con una sola read
	*/
	memset(response, 0, sizeof(response));
	/*approccio naive-> leggo fino a che non viene chiusa la socket lato server)*/
	while ((nread = read(sd, response, sizeof(response) - 1)) > 0){
		fprintf(stdout, "%s\n", response);
	}

	close(sd);

	return 0;
}
