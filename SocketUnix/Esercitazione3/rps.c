#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv){
  int sd, err;
  struct addrinfo hints, *ptr, *res;

  if(argc < 2 | argc > 3){
    fprintf(stderr, "Uso corretto: rps hostname porta <option>\n" );
    exit(1);
    
  }
  return 0;
}
