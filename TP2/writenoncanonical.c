/*Non-Canonical Input Processing*/


#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include "protocolo.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res=0;
    struct termios oldtio,newtio;
    char buf[255], buf_receive[255];
    int i, sum = 0, speed = 0;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp(MODEMDEVICE, argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    printf("New termios structure set\n");


    //sleep(20);

    //printf("Sai do Sleep, LL open");

    if( (fd = llopen(0,FLAG_LL_OPEN_TRANSMITTER)) < 0){
      printf("Error in llopen function\n");
      return -1;
    }

  /*
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
    o indicado no gui�o
  */

    int ret = 0;

    ret = llwrite(fd, "ola", 3);

    printf("Return: %d\n", ret);
    sleep(1);

    if(llclose(fd,FLAG_LL_CLOSE_RECEIVER_DISC)!=LL_CLOSE_SUCESS){
      printf("O LL close retornou erro\n");
      return -1;
    }
    
    return 0;
}
