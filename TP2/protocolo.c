#include "protocolo.h"

#define MAX_RETR 3
#define TIMEOUT 3

static unsigned int porta = 0;

void alarm_handler(int signo)
{
  if (signo != SIGALRM)
  {
    printf("Handler nao executado\n");
    return;
  }

  sendBlock(FLAG_LL_OPEN_TRANSMITTER);
  alarm(TIMEOUT);

  return;
}

int sendBlock(int flag)
{

  unsigned char buf[BUF_SIZE];

  //A Flag inicial e comum a qualquer trama
  if (flag == FLAG_LL_OPEN_RECEIVER || flag == FLAG_LL_OPEN_TRANSMITTER)
  {
    //E se usassemos esta funcao para depois enviarmos qualquer informacao que nao so LLOPEN?
    buf[FLAG_INDEX_BEGIN] = FLAG;
    buf[A_INDEX] = A_EM;

    if (flag == FLAG_LL_OPEN_TRANSMITTER)
    {
      buf[C_INDEX] = C_SET;
    }
    else
    {
      buf[C_INDEX] = C_UA;
    }

    buf[BCC_INDEX] = buf[A_INDEX] ^ buf[C_INDEX];

    buf[FLAG_INDEX_END] = FLAG;

    int bytes_send = write(porta, buf, BUF_SIZE);

    if (bytes_send != BUF_SIZE)
    {
      perror("Error writing in llopen:");
      return -1;
    }

    return WRITE_SUCCESS;
  }
  else
  {

    printf("SEND BLOCK not implemented\n");
    return WRITE_FAIL;
  }
}

int readBlock(int flag)
{

  unsigned char buf[MAX_BUF];
  unsigned int size = 0, state = ST_START;

  if (flag == FLAG_LL_OPEN_RECEIVER || flag == FLAG_LL_OPEN_TRANSMITTER)
  {

    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {

      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if (!read(porta, &buf[size], 1))
      {
        if (errno == EINTR)
        {
          printf("Recebi um sinal no intermedio\n");
          return READ_FAIL;
        }
        perror("Failled to read");
        return READ_FAIL;
      }

      switch (state)
      {
      case ST_START:
        //check FLAG byte
        if (buf[size] == FLAG)
          state = ST_FLAG_RCV;
        break;

      case ST_FLAG_RCV:

        switch (buf[size])
        {
        case A_EM:

          //Recebi uma mensagem do emissor ou um resposta do recetor
          state = ST_A_RCV;

          break;

        case FLAG:
          //Same state

          break;
        default:
          state = ST_START;
          break;
        }

      case ST_A_RCV:
        switch (buf[size])
        {
        case C_UA:

          if (flag == FLAG_LL_OPEN_TRANSMITTER)
            //received C_SET and is Transmitter, go to state C received
            state = ST_C_RCV;
          else
            //received C_SET and is Receiver, go to state start
            state = ST_START;
          break;

        case C_SET:
          if (flag == FLAG_LL_OPEN_RECEIVER)
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
        if (buf[size] == (A_EM ^ C_SET))
          //BCC correct
          state = ST_BCC_OK;
        else if (buf[size] == FLAG)
          //Received FLAG
          state = ST_FLAG_RCV;
        else
          //Received other
          state = ST_START;

        break;

      case ST_BCC_OK:
        //check FLAG byte
        if (buf[size] == FLAG)
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
    printf("Este return eu alterei para fail. Estava Sucess\n");
    return READ_FAIL;
  }

  else
  {
    printf("Read block not implemented\n");
    return READ_FAIL;
  }
}

int llopen(int fd, int flag)
{

  porta = fd;

  //Sending SET/UA frame
  if (flag == FLAG_LL_OPEN_TRANSMITTER)
  {

    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
      perror("Error instaling SIG ALARM handler\n");
      return -1;
    }

    if (sendBlock(FLAG_LL_OPEN_TRANSMITTER) != WRITE_SUCCESS)
    {
      printf("Error in sendSet function\n");
    }

    //Set alarm
    alarm(TIMEOUT);

    int ret_read_block = READ_FAIL;

    for (int tries = 0; tries < MAX_RETR; tries++)
    {

      ret_read_block = readBlock(FLAG_LL_OPEN_TRANSMITTER);

      if (ret_read_block == READ_SUCCESS)
      {
        break;
      }
    }

    if (ret_read_block == READ_FAIL)
    {
      printf("AS tentativas todas deram fail. Retornei erro\n");
      return READ_FAIL;
    }

    if (signal(SIGALRM, SIG_IGN) == SIG_ERR)
    {
      perror("Error in ignoring SIG ALARM handler");
    }
  }

  //Receives SET frame
  else if (flag == FLAG_LL_OPEN_RECEIVER)
  {

    if (readBlock(FLAG_LL_OPEN_RECEIVER) != READ_SUCCESS)
    {
      perror("Error in reading from llopen:");
      return -1;
    }

    if (sendBlock(FLAG_LL_OPEN_RECEIVER) != WRITE_SUCCESS)
    {
      perror("Error in writing from llopen:");
      return -1;
    }
  }

  //LL open deve retornar identificador da ligacao de dados
  return fd;
}
