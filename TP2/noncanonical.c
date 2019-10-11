/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include "protocolo.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    int porta;
    struct termios oldtio,newtio;
    unsigned char buf[255];
    char unit_char_received[2]; //2 positions because of
    int size=0;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    if((strcmp(MODEMDEVICE_0, argv[1])==0)){
      porta=0;

    }else{
      porta=1;
    }


    if((fd=llopen(porta,FLAG_LL_OPEN_RECEIVER))<0){
      printf("Erro no receiver no llopen\n");
      return -1;
    }


    sleep(10); //Resolve bug de receber lixo


    if(llclose(fd,FLAG_LL_CLOSE_RECEIVER_DISC)!=0){
      printf("Erro no llclose\n");
      return -1;

    }



    return 0;
}
