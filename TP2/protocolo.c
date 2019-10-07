#include "protocolo.h"

#define MAX_RETR 3
#define TIMEOUT 3

static struct termios oldtio;

int openNonCanonical(int port_number){
  struct termios newtio;
  int fd_port = 0;

  switch (port_number) {
    case 0:
      fd_port = open(MODEMDEVICE_0, O_RDWR | O_NOCTTY );
      break;
    case 1:
      fd_port = open(MODEMDEVICE_1, O_RDWR | O_NOCTTY );
      break;
    default:
      return UNKNOWN_PORT;
  }

  if (fd_port < 0) {
    perror("Error");
    exit(OTHER_ERROR);
  }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(OTHER_ERROR);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 char received */


  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(OTHER_ERROR);
  }

  return fd_port;
}

void alarm_handler(int signo)
{
  static int n_tries = MAX_RETR;

  if (signo != SIGALRM)
  {
    printf("Handler nao executado\n");
    return;
  }
  if(n_tries>0){
    n_tries--;
    sendBlock(FLAG_LL_OPEN_TRANSMITTER);
    alarm(TIMEOUT);
  }else{
    n_tries = MAX_RETR;
    llclose(fd);
  }

  return;
}

int sendBlock(int flag)
{

  unsigned char buf[BUF_SIZE+1];

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


    //Coloquei este \0 de proposito para ver se as vezes nao sera este o problema
    buf[BUF_SIZE]='\0';
    //

    int bytes_send = write(fd, buf, BUF_SIZE);

    if (bytes_send != BUF_SIZE)
    {
      perror("Error writing in llopen:");
      return WRITE_FAIL;
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

  unsigned char leitura;
  unsigned int size = 0, state = ST_START;

  if (flag == FLAG_LL_OPEN_RECEIVER || flag == FLAG_LL_OPEN_TRANSMITTER)
  {

    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {
      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if (!read(fd, &leitura, 1))
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
      case ST_START:{
        //check FLAG byte
        if (leitura == FLAG){
          state = ST_FLAG_RCV;
        }
      }
        break;

      case ST_FLAG_RCV:

        switch (leitura)
        {
        case A_EM:{
          //Recebi uma mensagem do emissor ou um resposta do recetor
          state = ST_A_RCV;
          break;
        }

        case FLAG:
          //Same state

          break;
        default:
          state = ST_START;
        }

        break;

      case ST_A_RCV:
        switch (leitura)
        {
        case C_UA:

          if (flag == FLAG_LL_OPEN_TRANSMITTER){
            //received C_SET and is Transmitter, go to state C received
            state = ST_C_RCV;
            break;

          }
          else
            //received C_SET and is Receiver, go to state start
            state = ST_START;

          break;

        case C_SET:

          if (flag == FLAG_LL_OPEN_RECEIVER){
            //received C_SET and is Receiver, go to state C received
            state = ST_C_RCV;
          }
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

      case ST_C_RCV:{

        //received BCC, check BCC

        if (leitura == (A_EM ^ C_SET)&&flag==FLAG_LL_OPEN_RECEIVER){

          //BCC correct
          state = ST_BCC_OK;

        }
        else if(leitura == (A_EM ^ C_UA)&&flag==FLAG_LL_OPEN_TRANSMITTER){
          state = ST_BCC_OK;
        }
        else if (leitura == FLAG){

          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else{
          //Received other
          state = ST_START;

        }

        break;

      }


      case ST_BCC_OK:
        //check FLAG byte
        if (leitura == FLAG){

          //received all, stop cycle
          return READ_SUCCESS;
        }
        else
          //received other, go to start
          state = ST_START;
        break;

      default:
        state = ST_START;
        break;
      }
    }
    return READ_FAIL;
  }

  else
  {
    printf("Read block not implemented\n");
    return READ_FAIL;
  }
}

int llopen(int port_number, int flag)
{
  int fd;

  fd = openNonCanonical(port_number);

  if(fd == UNKNOWN_PORT){
    printf("Unknown port, must be either 0 or 1\n");
    exit(UNKNOWN_PORT);
  }


  //Transmitter
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

    while(n_tries>0&&ret_read_block==READ_FAIL){
      ret_read_block = readBlock(FLAG_LL_OPEN_TRANSMITTER);
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

  //Receiver
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

int llclose(int fd){

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  if(close(fd)!=0){
    perror("Failled to close file");
  }

  return 0;
}
