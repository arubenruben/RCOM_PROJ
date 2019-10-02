#include "protocolo.h"

#define MAX_RETR 3
#define TIMEOUT 3

bool finish = false;
int retry_num = 0;
static unsigned int porta = 0;
unsigned int n_bytes = 0;

void alarm_handler(int signo) {

  while(!finish) {

    if(retry_num < MAX_RETR) {
      if(sendBlock(FLAG_LL_OPEN_TRANSMITTER) != READ_SUCCESS) {
        return;
      }

      alarm(TIMEOUT);
      retry_num++;
    }

    else {
      finish = true;
    }
  }


  return;
}

int sendBlock(int flag) {
  unsigned char buf[5];

  buf[FLAG_INDEX_BEGIN] = FLAG;
  if(flag == FLAG_LL_OPEN_TRANSMITTER){
    buf[A_INDEX] = A_EM;
    buf[C_INDEX] = C_SET;
  }
  else{
    buf[A_INDEX] = A_RE;
    buf[C_INDEX] = C_UA;
  }
  buf[BCC_INDEX] = buf[A_INDEX] ^ buf[C_INDEX];
  buf[FLAG_INDEX_END] = FLAG;

  int bytes_send = write(porta, buf, BUF_SIZE);

  if(bytes_send != BUF_SIZE) {
    perror("Error writing in llopen:");
    return -1;
  }

  return bytes_send;
}

int readBlock(int flag){
  unsigned char buf[MAX_BUF];
  unsigned int size = 0, state = ST_START;

  if(flag != FLAG_LL_OPEN_RECEIVER || flag != FLAG_LL_OPEN_TRANSMITTER)
    return INVALID_PARAMS;

  for (size = 0; state != ST_STOP && size < MAX_BUF; size++){

    //read byte
    if (!read(porta, &buf[size], 1)){
      if(errno == EINTR){
        continue;
        state = ST_START;
      }
      perror("Failled to read");
      return READ_FAIL;
    }

    switch (state){
      case ST_START:
        //check FLAG byte
        if(buf[size] == FLAG)
          state = ST_FLAG_RCV;
        break;

      case ST_FLAG_RCV:
        switch (buf[size]){
          case A_RE:
            if(flag == FLAG_LL_OPEN_TRANSMITTER)
              //received A and is Transmitter, go to state A received
              state = ST_A_RCV;
            else
              //received A and is Receiver, go to state start
              state = ST_START;
            break;

          case A_EM:
            if(flag == FLAG_LL_OPEN_RECEIVER)
              //received A and is Receiver, go to state A received
              state = ST_A_RCV;
            else
              //received A and is Transmitter, go to state start
              state = ST_START;
            break;

          case FLAG:
            //received FLAG, same state
            break;

          default:
            //received other, go to start
            state = ST_START;
            break;
        }
        break;

      case ST_A_RCV:
        switch (buf[size]){
          case C_UA:
            if(flag == FLAG_LL_OPEN_TRANSMITTER)
              //received C_SET and is Transmitter, go to state C received
              state = ST_C_RCV;
            else
              //received C_SET and is Receiver, go to state start
              state = ST_START;
            break;

          case C_SET:
            if(flag == FLAG_LL_OPEN_RECEIVER)
              //received C_SET and is Receiver, go to state C received
              state = ST_C_RCV;
            else
              //received C_SET and is Transmitter, go to state start
              state = ST_START;
            break;

          case FLAG:
            //received FLAG, go to flag state
            state = ST_FLAG_RCV;
            break;

          default:
            //received other, go to start
            state = ST_START;
            break;
        }
        break;

      case ST_C_RCV:
        //received BCC, check BCC
        if(flag == FLAG_LL_OPEN_RECEIVER){
          if(buf[size] == (A_EM^C_SET))
          //BCC correct
          state = ST_BCC_OK;
          else if(buf[size] == FLAG)
          //Received FLAG
          state = ST_FLAG_RCV;
          else
          //Received other
          state = ST_START;
        }
        else{
          if(buf[size] == (A_RE^C_UA))
          //BCC correct
          state = ST_BCC_OK;
          else if(buf[size] == FLAG)
          //Received FLAG
          state = ST_FLAG_RCV;
          else
          //Received other
          state = ST_START;
        }
        break;

      case ST_BCC_OK:
          //check FLAG byte
          if(buf[size] == FLAG)
            //received all, stop cycle
            return READ_SUCCESS;
          else
            //received other, go to start
            state = ST_START;
        break;

      default:
        state = ST_START;
        break;
    }
  }

  return READ_SUCCESS;
}

int llopen(int fd, int flag) {

  porta = fd;
/*
  if(flag == FLAG_LL_OPEN_TRANSMITTER) {
    buf[C_INDEX] = C_SET;
  }

  else if(flag == FLAG_LL_OPEN_RECEIVER) {
    buf[C_INDEX] = C_UA;
  }

  else {
    printf("Wrong flag value\n");
    return -1;
  }

  buf[FLAG_INDEX_BEGIN] = FLAG;
  buf[A_INDEX] = A_EM;
  buf[BCC_INDEX] = buf[A_INDEX] ^ buf[C_INDEX];
  buf[FLAG_INDEX_END] = FLAG;
*/
  //Sending SET/UA frame
  if(flag == FLAG_LL_OPEN_TRANSMITTER) {

    if(signal(SIGALRM, alarm_handler) == SIG_ERR) {
      perror("Error instaling SIG ALARM handler\n");
      return -1;
    }

    if(sendBlock(FLAG_LL_OPEN_TRANSMITTER) != BUF_SIZE) {
      printf("Error in sendSet function\n");
    }

    //Set alarm
    alarm(TIMEOUT);

    if(readBlock(FLAG_LL_OPEN_RECEIVER) != BUF_SIZE) {
      perror("Error reading from llopen\n");
      return -1;
    }

    if(signal(SIGALRM, SIG_IGN) == SIG_ERR) {
      perror("Error in ignoring SIG ALARM handler");
    }
  }

  //Receives  SET/UA frame
  else if(flag == FLAG_LL_OPEN_RECEIVER) {

    if(read(porta, buf, sizeof(buf)) != BUF_SIZE) {
      perror("Error in reading from llopen:");
      return -1;
    }

    if(signal(SIGALRM, SIG_IGN) == SIG_ERR) {
      perror("Error in ignoring SIG ALARM handler");
    }

    if(write(porta, buf, sizeof(buf)) != BUF_SIZE) {
      perror("Error in writing from llopen:");
      return -1;
    }

    if(signal(SIGALRM, alarm_handler) == SIG_ERR) {
      perror("Error instaling SIG ALARM handler\n");
      return -1;
    }
  }
  return porta;
}
