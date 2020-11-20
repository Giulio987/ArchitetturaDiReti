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
#include "utils.h"

int main(void)
{
  int sd, err;
  char buff[2048];
  struct addrinfo hints, *res;

  memset(&hints, 0, sizeof(hints));
  /* Usa AF_INET per forzare solo IPv4, AF_INET6 per forzare solo IPv6 */
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // Permette alla gettaddrinfo di andare a funzionare in modalità passiva
  //preparandoci la struttura dati da passare all bind
  //Non proprio portabilissimo se si usa AF_UNSPEC-> BSD e Windows-> poiche la situazione migliore sarebbe
  //di avere 2 socket una AF_INET e una AF_INET6-> molto piu complicato e servirebbe solo per avere compatibilità massima
  if ((err = getaddrinfo(NULL, "50001", &hints, &res)) != 0)
  {
    fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
    exit(EXIT_FAILURE);
  }
  //Oviiamente a lato server non si usa il fallback-> si aggancia a uno dei due protoclli senza dire niente(IPV6 senza saperlo)
  if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
  {
    perror("Errore in socket");
    exit(EXIT_FAILURE);
  }

  if (bind(sd, res->ai_addr, res->ai_addrlen) < 0)
  {
    perror("Errore in bind");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(res);

  /* trasforma in socket passiva d'ascolto */
  if (listen(sd, SOMAXCONN) < 0)
  { //SOMAXCONN è la lunghezza della coda
    perror("Errore in listen");
    exit(EXIT_FAILURE);
  }

  /* entro nella directory */
  /* chdir("/var/local/files"); */

  for (;;)
  {
    int fd, ns, nread;
    struct sockaddr_storage client_address; // struttura pensata proprio per accept-Z in grado
    // di contenere tutte le strutture che ritornano
    socklen_t fromlen;

    fromlen = sizeof(client_address);
    //accept riceve (porta passiva, indirizzo di memoria su cui andrà a scrivere l'estremo lato client)
    //ogni volta che la chiamo devi resetta fromlen

    ns = accept(sd, (struct sockaddr *)&client_address, &fromlen);
    //Se non ci interessa leggere l'indirizzo del client passo-> sd, NULL, NULL
    /* ns = accept(sd, NULL, NULL); */
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
    //L'assunzione è che con una sola read si legga tutta la stringa(separata in pacchetti tcp)-> non si dovrebbe fare
    if ((nread = read(ns, buff, sizeof(buff) - 1)) < 0)
    {
      perror("Errore in read");
      close(ns);
      continue;
    }
    //passo il buffer come stringa null terminated
    printf("Il client ha chiesto il file %s.\n", buff);
    if ((fd = open(buff, O_RDONLY)) < 0)
    {
      printf("File non esiste.\n");
      if (write(ns, "N", 1) < 0)
      {
        perror("Errore in write");
        close(ns);
        continue;
      }
    }
    else
    {
      int cont = 0;

      printf("File esiste, lo invio al client.\n");
      if (write(ns, "S", 1) < 0)
      {
        perror("Errore in write");
        close(ns);
        continue;
      }
      while ((nread = read(fd, buff, sizeof(buff))) > 0)
      {
        if (write_all(ns, buff, nread) < 0)
        { //Vogliamo essere sicuri che tutti i caratteri vengano scritti
          perror("Errore in write");
          close(ns);
          continue;
        }
        cont += nread;
      }
      printf("Copia eseguita di %d byte.\n", cont);
      close(fd);
    }
    close(ns);
  }

  return 0;
}
