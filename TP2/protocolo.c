#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "flags.h"
#include "structs.h"
#include "protocolo.h"


int llopen(int porta, int flag){


  unsigned char buf[5];

  buf[FLAG_INDEX_BEGIN]=FLAG;
  buf[A_INDEX]=A_EM;


  if(flag==FLAG_LL_OPEN_TRANSMITTER){

    buf[C_INDEX]=C_SET;

  }else if(flag==FLAG_LL_OPEN_RECEIVER){
    buf[C_INDEX]=C_UA;


  }else{
    printf("Erro a na funcao LL OPEN");
    return -1;
  }

  buf[BCC_INDEX]=buf[A_INDEX]^buf[C_INDEX];
  buf[FLAG_INDEX_END]=FLAG;


  //Envio de SET/UA FRAME

  if(flag==FLAG_LL_OPEN_TRANSMITTER){

    if(write(porta,buf,sizeof(buf))!=BUF_SIZE){
      perror("Erro no write de llopen:");
      return -1;
    }

    if(read(porta,buf,sizeof(buf))!=BUF_SIZE){

      perror("Erro no read de llopen:");
      return -1;
    }

  }else if(flag==FLAG_LL_OPEN_RECEIVER){

    if(read(porta,buf,sizeof(buf))!=BUF_SIZE){

      perror("Erro no read de llopen:");
      return -1;
    }

    if(write(porta,buf,sizeof(buf))!=BUF_SIZE){
      perror("Erro no write de llopen:");
      return -1;
    }

  }


  //Recebe o SET/UA FRAME

  return porta;
}
