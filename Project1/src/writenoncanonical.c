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
    int fd, port_number;
    char buf[255] = "\n\n\n\n\nVIVA O FUTEBOL CLUBE DO PORTO\n\n\n\n\n\n";

    if ( (argc < 2) ||
  	     ((strcmp(MODEMDEVICE_0, argv[1])!=0) &&
  	      (strcmp(MODEMDEVICE_1, argv[1])!=0)&&(strcmp(MODEMDEVICE_2, argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    if(strcmp(MODEMDEVICE_0, argv[1]) == 0) {
        port_number = 0;
    }

    else if(strcmp(MODEMDEVICE_1, argv[1]) == 0) {
        port_number = 1;
    }

    else {
        port_number = 2;
    }

    if((fd = llopen(port_number, FLAG_LL_OPEN_TRANSMITTER)) < 0) {
        printf("Error in llopen!\n");
        return -1;
    }

    sleep(3);

    if(llwrite(fd, (unsigned char*)buf, strlen(buf)) != 0) {
        printf("Error in llwrite!\n");
        return -1;
    }

    if(llclose(fd,FLAG_LL_CLOSE_TRANSMITTER_DISC)!=LL_CLOSE_SUCESS){
      printf("Error in llclose!\n");
      return -1;
    }

    return 0;
}
