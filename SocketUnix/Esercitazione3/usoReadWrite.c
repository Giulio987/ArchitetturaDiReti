int main(int argc, char **argv)
{
    int sd;
    char buff[4096];
    char *p = buff;


    strcopy(buff, "messaggio");
  /*
  * sd = socket...
  * buff = {'m', 'e','s','s','a','g','g','i','o','0x00',....}
  * sizoef(buff) = 4096
  * strlen(buff) = 9
  * strlen(p) = 9
  * sizeof(p) = 4 su macchine a 32 bit e 8 su macchine a 64
  * sizeof(argv[X]) = 4 su macchine a 32 bit e 8 su macchine a 64
  * 
  * Volendo si potrebbe inviare con
  * write(sd, buff, strlen(buff) + 1) se sono sicuro 
  * di avere il terminatore e quindi invio anche quello
  * quindi alla ricezione non dovrò mettere sizeof(buff) -1
  */
    return 0;
}