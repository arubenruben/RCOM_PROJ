#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "flags.h"
#include "structs.h"
#include "protocolo.h"

#define MAX_RETR 3
#define TIMEOUT 3


int finish=0,num_retr=0;

void alarm_handler(int signo){

  if(num_retr<MAX_RETR){
    send_Set();
    alarm(TIMEOUT);
    num_retr++;

  }else{
    finish=1;
  }

  return;
}

int send_Set(int porta, unsigned char *buf,unsigned int n_bytes){

  int bytes_send=0;
  bytes_send=write(porta,buf,n_bytes);

  if(bytes_send!=BUF_SIZE){
    perror("Erro no write de llopen:");
    return -1;
  }

  return bytes_send;
}


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

    if(send_Set(porta,buf,sizeof(buf)<BUF_SIZE){
      printf("Erro em send_set");
    }


    if(signal(SIGALARM,alarm_handler)==SIGERR){
      perror("Erro a instalar handler para o timeout:");
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
