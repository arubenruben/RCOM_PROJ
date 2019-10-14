#include "protocolo.h"

#define MAX_RETR 3
#define TIMEOUT 3

void alarm_handler_set_signal(int signo);
static int fd_for_handler;
static int type_handling=HANDLING_UNDEFINED;

int sendBlock( int flag,  int fd);
int readBlock( int flag,  int fd);

static struct termios oldtio;
static int n_tries = MAX_RETR;

DataStruct createMessage(unsigned int sequenceNumber, char *buffer, int length);
DataStruct *pointer_to_data=NULL;
unsigned int BCC2Stufying(unsigned char *BCC2);
unsigned int dataStuffing(unsigned char *data, int length, unsigned char *fieldD);

int openNonCanonical(int port_number)
{
  struct termios newtio;

  int fd_port = 0;

  switch (port_number)
  {
  case 0:
    fd_port = open(MODEMDEVICE_0, O_RDWR | O_NOCTTY);
    break;
  case 1:
    fd_port = open(MODEMDEVICE_1, O_RDWR | O_NOCTTY);
    break;
  case 2:
    fd_port = open(MODEMDEVICE_2, O_RDWR | O_NOCTTY);
  break;
  default:
    return UNKNOWN_PORT;
  }

  if (fd_port < 0)
  {
    perror("Error");
    exit(OTHER_ERROR);
  }

  if (tcgetattr(fd_port, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(OTHER_ERROR);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0;              /* inter-character timer unused */
  newtio.c_cc[VMIN] = LER_BYTE_A_BYTE; /* blocking read until 1 char received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */

  tcflush(fd_port, TCIOFLUSH);

  if (tcsetattr(fd_port, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(OTHER_ERROR);
  }

  return fd_port;
}

void alarm_handler_set_signal(int signo)
{

  printf("Alarme chamado\n");

  if (signo != SIGALRM)
  {
    printf("Handler nao executado\n");
    return;
  }
  if (n_tries > 0)
  {
    if (sendBlock(FLAG_LL_OPEN_TRANSMITTER, fd_for_handler) != WRITE_SUCCESS)
    {
      printf("Error in sendSet function no handler\n");
    }
    alarm(TIMEOUT);
    n_tries--;
  }
  else
  {
    //printf("Handler chama llclose.TIMEOUT TOTAL\n");
    llclose(fd_for_handler, FLAG_HANDLER_CALL);
    exit(-1);
  }

  return;
}

void alarm_handler_disc_signal(int signo)
{

  if (signo != SIGALRM)
  {
    printf("Handler nao executado\n");
    return;
  }
  
  if (n_tries > 0)
  {
    if(type_handling==HANDLING_CLOSE_EMISSOR){
      if (sendBlock(FLAG_LL_CLOSE_TRANSMITTER_DISC,fd_for_handler) != WRITE_SUCCESS)
      {
        printf("Error in sendSet function no handler\n");
      }

    }else if(type_handling==HANDLING_CLOSE_RECETOR){

      if (sendBlock(FLAG_LL_CLOSE_RECEIVER_DISC,fd_for_handler) != WRITE_SUCCESS)
      {
        printf("Error in sendSet function no handler\n");
      }
    }else{
      printf("Handling undefined. STOP\n");
      exit(-1);
    }

    alarm(TIMEOUT);
    n_tries--;
  }
  else
  {
    printf("No LLCLOSE fui incapaz de desligar corretamente\nMatei o processo\n");
    exit(-1);
  }

  return;
}

void alarm_handler_data(int signo)
{
  printf("Alarme\n");
  if (signo != SIGALRM)
  {
    printf("Handler nao executado\n");
    return;
  }
  
  if (n_tries > 0)
  {
    sendBlock(FLAG_LL_DATA_SEND,fd_for_handler);

    alarm(TIMEOUT);
    n_tries--;
  }
  else
  {
    printf("No LLCLOSE fui incapaz de desligar corretamente\nMatei o processo\n");
    exit(-1);
  }

  return;
}

int checkBCC2(unsigned char *buffer, unsigned int size)
{
  unsigned char bcc = buffer[size - 2];
  unsigned char bcc_check = 0;

  for (int i = 4; i < (size - 2); i++)
  {
    bcc_check ^= buffer[size];
  }

  return bcc == bcc_check;
}

int byteDeStuffing(unsigned char *dest, unsigned char *orig, unsigned int size_orig)
{
  int size_dest = 0;

  dest = (unsigned char *)malloc(size_orig * sizeof(unsigned char *));
  if (dest == NULL)
  {
    perror("Failled to allocate memory");
    exit(NO_MEM);
  }

  for (int i = DATA_START_INDEX; i < (size_orig - 1); i++)
  {
    if (orig[i] == ESC)
    {
      i++;
      if (orig[i] == ESC_FLAG)
        dest[size_dest] = FLAG;
      else
        dest[size_dest] = ESC;
    }
    else
      dest[size_dest] = orig[i];

    size_dest++;
  }

  return size_dest;
}

int sendBlock(int flag, int fd)
{
  printf("Enviei:");
  
  unsigned char buf[BUF_SIZE + 1];
  
  //A Flag inicial e comum a qualquer trama
  if (flag == FLAG_LL_OPEN_RECEIVER || flag == FLAG_LL_OPEN_TRANSMITTER)
  {
    //E se usassemos esta funcao para depois enviarmos qualquer informacao que nao so LLOPEN?
    buf[FLAG_INDEX_BEGIN] = FLAG;
    buf[A_INDEX] = A_CE_AR;

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

    int bytes_send = write(fd, buf, BUF_SIZE);

    if (bytes_send != BUF_SIZE)
    {
      perror("Error writing in llopen:");
      return WRITE_FAIL;
    }
  }

  else if (flag == FLAG_LL_CLOSE_TRANSMITTER_DISC || flag == FLAG_LL_CLOSE_TRANSMITTER_UA)
  {

    buf[FLAG_INDEX_BEGIN] = FLAG;

    buf[A_INDEX] = A_CE_AR;

    if (flag==FLAG_LL_CLOSE_TRANSMITTER_DISC)
    {
      printf("Preenchi com disc-- Ua\n");
      buf[C_INDEX] = C_DISC;
    }
    else if (flag==FLAG_LL_CLOSE_TRANSMITTER_UA)
    {
      buf[C_INDEX] = C_UA;
    }

    buf[BCC_INDEX] = buf[A_INDEX] ^ buf[C_INDEX];

    buf[FLAG_INDEX_END] = FLAG;

    if (write(fd, buf, BUF_SIZE) != BUF_SIZE)
    {
      perror("Erro no write do FLAG_LL_CLOSE_TRANSMITTER_DISC:");
      return WRITE_FAIL;
    }

  }

  else if (flag == FLAG_LL_CLOSE_RECEIVER_DISC)
  {
    buf[FLAG_INDEX_BEGIN] = FLAG;

    buf[A_INDEX] = A_CR_AE;

    if (FLAG_LL_CLOSE_RECEIVER_DISC)
    {
      buf[C_INDEX] = C_DISC;
    }
    
    buf[BCC_INDEX] = buf[A_INDEX] ^ buf[C_INDEX];

    buf[FLAG_INDEX_END] = FLAG;

    if (write(fd, buf, BUF_SIZE) != BUF_SIZE)
    {
      perror("Erro no write do FLAG_LL_CLOSE_TRANSMITTER_DISC:");
      return WRITE_FAIL;
    }
  }
  else if(flag==FLAG_LL_DATA_SEND)
  {
    int n_bytes=-1;

    n_bytes=write(fd,pointer_to_data,pointer_to_data->size_of_data_frame);
  
    if(n_bytes<0){
    
      printf("Nao escrevi o bloco de data todo");
      return WRITE_FAIL;

    }else{
      printf("Retornei sucesso do ll_data_send\n");
      for(int i=0;i<3;i++){
        printf("%c:\n",pointer_to_data->fieldD[i]);
      }
      return n_bytes;
    }


  }
  else
  {
    printf("SEND BLOCK not implemented\n");
    return WRITE_FAIL;
  }

  if(flag!=FLAG_LL_DATA_SEND){

    for(int j=0;j<BUF_SIZE;j++){
     printf("%x ",buf[j]);
    }

    printf("\n");

  }

  return WRITE_SUCCESS;
}

int readBlock(int flag, int fd)
{

  unsigned char leitura;
  unsigned int size = 0, state = ST_START;
  printf("\nRecebi:\n");


  if (flag == FLAG_LL_OPEN_RECEIVER || flag == FLAG_LL_OPEN_TRANSMITTER  || flag == FLAG_LL_CLOSE_TRANSMITTER_UA || flag == FLAG_LL_CLOSE_RECEIVER_DISC)
  {

    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {
      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if ((read(fd, &leitura, 1) != 0))
      {
        printf("%x ",leitura);
      }

      switch (state)
      {
      case ST_START:
      {
        //check FLAG byte
        if (leitura == FLAG)
        {
          state = ST_FLAG_RCV;
        }
      }
      break;

      case ST_FLAG_RCV:

        switch (leitura)
        {
        case A_CE_AR:
        {
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

          if (flag == FLAG_LL_OPEN_TRANSMITTER || flag == FLAG_LL_CLOSE_RECEIVER_UA)
          {
            //received C_SET and is Transmitter, go to state C received
            //Or its llclose receiver a receiver
            state = ST_C_RCV;
            break;
          }
          else
            //received C_SET and is Receiver, go to state start
            state = ST_START;

          break;

        case C_SET:

          if (flag == FLAG_LL_OPEN_RECEIVER)
          {
            //received C_SET and is Receiver, go to state C received
            state = ST_C_RCV;
          }
          else
            //received C_SET and is Transmitter, go to state start
            state = ST_START;
          break;

        case C_DISC:

          if (flag == FLAG_LL_CLOSE_RECEIVER_DISC || flag == FLAG_LL_CLOSE_TRANSMITTER_DISC)
          {
            //received UA on LL_CLOSE
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

      case ST_C_RCV:
      {

        //received BCC, check BCC

        if (leitura == (A_CE_AR ^ C_SET) && flag == FLAG_LL_OPEN_RECEIVER)
        {

          //BCC correct
          state = ST_BCC_OK;
        }
        else if (leitura == (A_CE_AR ^ C_UA) && flag == FLAG_LL_OPEN_TRANSMITTER)
        {
          state = ST_BCC_OK;
        }
        else if (leitura == (A_CE_AR ^ C_DISC) && flag == FLAG_LL_CLOSE_RECEIVER_DISC)
        {
          state = ST_BCC_OK;
        }
        else if (leitura == (A_CR_AE ^ C_DISC) && flag == FLAG_LL_CLOSE_TRANSMITTER_DISC)
        {
          state = ST_BCC_OK;
        }
        else if (leitura == FLAG)
        {

          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_BCC_OK:
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
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
  else if(flag==FLAG_LL_CLOSE_TRANSMITTER){

    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {
      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if ((read(fd, &leitura, 1) != 0))
      {
        printf("%x ",leitura);
      }

      switch (state)
      {
      case ST_START:
      {
        //check FLAG byte
        if (leitura == FLAG)
        {
          state = ST_FLAG_RCV;
        }
      }
      break;

      case ST_FLAG_RCV:

        switch (leitura)
        {
        case A_CR_AE:
        {
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
        case C_DISC:

          state = ST_C_RCV;
          
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
      {

        //received BCC, check BCC

        if (leitura == (A_CR_AE ^ C_DISC))
        {
          state = ST_BCC_OK;
        }

        else if (leitura == FLAG)
        {
          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_BCC_OK:
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
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
  else if(flag == FLAG_LL_CLOSE_RECEIVER_UA){
  
    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {
      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if ((read(fd, &leitura, 1) != 0))
      {
        printf("%x ",leitura);
      }

      switch (state)
      {
      case ST_START:
      {
        //check FLAG byte
        if (leitura == FLAG)
        {
          state = ST_FLAG_RCV;
        }
      }
      break;

      case ST_FLAG_RCV:

        switch (leitura)
        {
        case A_CE_AR:
        {
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

          state = ST_C_RCV;
          
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
      {

        //received BCC, check BCC

        if (leitura == (A_CE_AR ^ C_UA))
        {
          state = ST_BCC_OK;
        }

        else if (leitura == FLAG)
        {
          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_BCC_OK:
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
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

  else if(flag==FLAG_DATA_SEEKING_ANSWER_WITH0){

    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {
      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if ((read(fd, &leitura, 1) != 0))
      {
        printf("Li do receiver:%x\n ",leitura);
      }

      switch (state)
      {
      case ST_START:
      {
        //check FLAG byte
        if (leitura == FLAG)
        {
          state = ST_FLAG_RCV;
        }
      }
      break;

      case ST_FLAG_RCV:

        switch (leitura)
        {
        case A_CE_AR:
        {
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
        case C_REJ(0):

          state = ST_C_RCV_REJ;
          
        break;

        case C_RR(0):

          state=ST_C_RCV_RR;

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

      case ST_C_RCV_REJ:
      {

        //received BCC, check BCC

        if (leitura == (A_CE_AR ^ C_REJ(0)))
        {
          state = ST_BCC_OK;
        }

        else if (leitura == FLAG)
        {
          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_C_RCV_RR:
      {

        //received BCC, check BCC

        if (leitura == (A_CE_AR ^ C_RR(0)))
        {
          state = ST_BCC_OK;
        }

        else if (leitura == FLAG)
        {
          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_BCC_OK_REJ:

      
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
          return READ_REJ_SUCESS;
        }
        else
          //received other, go to start
          state = ST_START;
        break;
  
      case ST_BCC_OK_RR:
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
          return READ_RR_SUCESS;
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
  
  
  else if(flag==FLAG_DATA_SEEKING_ANSWER_WITH1){

    for (size = 0; state != ST_STOP && size < MAX_BUF; size++)
    {
      //A mensagem vai ser lida byte a byte para garantir que nao há falha de informacao

      if ((read(fd, &leitura, 1) != 0))
      {
        printf("lI DE RESPOSTA DO receiver%x\n ",leitura);
      }

      switch (state)
      {
      case ST_START:
      {
        //check FLAG byte
        if (leitura == FLAG)
        {
          state = ST_FLAG_RCV;
        }
      }
      break;

      case ST_FLAG_RCV:

        switch (leitura)
        {
        case A_CE_AR:
        {
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
        case C_REJ(1):

          state = ST_C_RCV_REJ;
          
        break;

        case C_RR(1):

          state=ST_C_RCV_RR;

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

      case ST_C_RCV_REJ:
      {

        //received BCC, check BCC

        if (leitura == (A_CE_AR ^ C_REJ(1)))
        {
          state = ST_BCC_OK;
        }

        else if (leitura == FLAG)
        {
          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_C_RCV_RR:
      {

        //received BCC, check BCC

        if (leitura == (A_CE_AR ^ C_RR(1)))
        {
          state = ST_BCC_OK;
        }

        else if (leitura == FLAG)
        {
          //Received FLAG
          state = ST_FLAG_RCV;
        }
        else
        {
          //Received other
          state = ST_START;
        }

        break;
      }

      case ST_BCC_OK_REJ:

      
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
          return READ_REJ_SUCESS;
        }
        else
          //received other, go to start
          state = ST_START;
        break;
  
      case ST_BCC_OK_RR:
        //check FLAG byte
        if (leitura == FLAG)
        {
          //received all, stop cycle
          printf("\nSucesso\n");
          return READ_RR_SUCESS;
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
  fd_for_handler=fd;
  int ret_read_block = READ_FAIL;
  n_tries=MAX_RETR;

  if (fd == UNKNOWN_PORT)
  {
    printf("Unknown port, must be either 0 or 1\n");
    exit(UNKNOWN_PORT);
  }

  //Transmitter
  if (flag == FLAG_LL_OPEN_TRANSMITTER)
  {

    if (signal(SIGALRM, alarm_handler_set_signal) == SIG_ERR)
    {
      perror("Error instaling SIG ALARM handler\n");
      return -1;
    }

    if (sendBlock(FLAG_LL_OPEN_TRANSMITTER, fd) != WRITE_SUCCESS)
    {
      printf("Error in sendSet function\n");
    }

    alarm(TIMEOUT);

    while (ret_read_block == READ_FAIL)
    {
      ret_read_block = readBlock(FLAG_LL_OPEN_TRANSMITTER, fd);
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
    if (readBlock(FLAG_LL_OPEN_RECEIVER, fd) != READ_SUCCESS)
    {
      perror("Error in reading from llopen:");
      return -1;
    }

    if (sendBlock(FLAG_LL_OPEN_RECEIVER, fd) != WRITE_SUCCESS)
    {
      perror("Error in writing from llopen:");
      return -1;
    }
  }

  //LL open deve retornar identificador da ligacao de dados
  printf("LLOpen completed\n");
  return fd;
}



/*LLwrite Enviar uma mensagem do emissor para o recetor


-Tem de preencher o bloco: Com o BCC calc
-Tem de fazer stuffing
-Tem de enviar tudo
-Tem de ficar a espera da resposta: Se for REJ Envio outra vez (vou enivar para ja tudo. Depois vejo como vamos para llread)

//So BCC2 e DADOS sao tidos em conta para stuffing. Os outros codigos estao escolhidos para nao ocorrer mal

*/

int llwrite(int fd, char *buffer, int length)
{


  //Nao esquecer que elas sao sempre cruzados envio 0 recebo 1. Comeco a 0
  static unsigned int sequenceNumber = 0;
  int num_bytes = 0;
  int ret_resposta=READ_FAIL;
  n_tries=MAX_RETR;

  
  DataStruct data = createMessage(sequenceNumber, buffer, length);
  data.size_of_data_frame=sizeof(data);
  pointer_to_data=&data;


  
  if (signal(SIGALRM, alarm_handler_data) == SIG_ERR)
  {
    perror("Error in ignoring SIG ALARM handler");
  }

  
  while(ret_resposta=READ_FAIL||ret_resposta==READ_REJ_SUCESS){
    printf("n vezes\n");

    num_bytes=sendBlock(FLAG_LL_DATA_SEND,fd);
    
    if(num_bytes==WRITE_FAIL){
      printf("Erro a enviar o bloco\n");
      return -1;
    }

    alarm(TIMEOUT);
    
    if(sequenceNumber==0){

      ret_resposta=readBlock(FLAG_DATA_SEEKING_ANSWER_WITH1,fd);


    }else if(sequenceNumber==1){

      ret_resposta=readBlock(FLAG_DATA_SEEKING_ANSWER_WITH0,fd);

    }

  }

  if (signal(SIGALRM, SIG_IGN) == SIG_ERR)
  {
      perror("Error in ignoring SIG ALARM handler");
  }


  free(data.fieldD);
  free(data.fieldBCC2);

  sequenceNumber = (sequenceNumber + 1) % 2;

  return num_bytes;

  
}

/*

Em LL read:

-Recebemos a mensagem
-Verificamos o cabecalho.
Dependendo do valor avancamos ou nao para o processamento dos dados

Nao esta certo: Estado morto. Timeout

Esta certo:

Fazer o distuffing

Testar Bcc2. 

Testar se a trama e repetida ou nao

Se repetida -> envio um RR nao passo nada à app
Se nova -> Passo a app e envio a app

----> Tambem se aplica ao caso de cabecalho certo, dados errados pq a seriacao da trama vem no C_VALUE


*/



int llread(int fd, char *buffer)
{
  static unsigned int r = 0;

  
  
  unsigned int size_buf = 0, state = ST_START, max_size = MAX_BUF;
  bool error = false;
  unsigned char *buf = NULL;
  unsigned char answer = C_REJ(r);
  int size_buffer = 0;

  if (buffer == NULL || fd < 0)
  {
    printf("Passei parametros invalidos a llread\n");
    return INVALID_PARAMS;
  }

  buf = (unsigned char *)malloc(max_size * sizeof(unsigned char));

  if (buf == NULL)
  {
    perror("Failled to allocate memory");
    exit(NO_MEM);
  }

  //While data is rejected be cause of errors go to state machine
  while (answer == C_REJ(r))
  {

    //State machine
    for (; state != ST_STOP; size_buf++)
    {
      //Check size
      if (size_buf > max_size)
      {
        max_size *= 2;
        buf = (unsigned char *)realloc((void *)buf, max_size * sizeof(unsigned char));

        if (buf == NULL)
        {
          perror("Failled to allocate memory");
          exit(NO_MEM);
        }
      }

      //Read byte
      if (!read(fd, &buf[size_buf], 1))
      {
        free(buf);
        perror("Failled to read");
        return READ_FAIL;
      }

      printf("%x\n",buf[size_buf]);
      continue;

      //Go through state machine
      switch (state)
      {

      case ST_START:
      {

        if (buf[size_buf] == FLAG)
          state = ST_FLAG_RCV;
        else
          size_buf = START_INDEX;
      }

      break;

      case ST_FLAG_RCV:
      {

        if (buf[size_buf] == A_CE_AR)
          state = ST_A_RCV;

        else if (buf[size_buf] != FLAG)
        {
          state = ST_START;
          size_buf = START_INDEX;
        }

        else
          size_buf--;
      }

      break;

      case ST_A_RCV:
      {

        if (buf[size_buf] == C(r))
          state = ST_C_RCV;

        else if (buf[size_buf] == FLAG)
        {
          state = ST_FLAG_RCV;
          size_buf -= 2;
        }

        else
        {
          state = ST_START;
          size_buf = START_INDEX;
        }
      }
      break;

      case ST_C_RCV:
      {

        if (buf[size_buf] == (A_CE_AR ^ C(r)))
          state = ST_BCC_OK;

        else if (buf[size_buf] == FLAG)
        {
          state = ST_FLAG_RCV;
          size_buf -= 3;
        }
        else
        {
          state = ST_START;
          size_buf = START_INDEX;
        }
      }
      break;

      case ST_D:
      {

        if (buf[size_buf] == FLAG)
          state = ST_STOP;
        else if (buf[size_buf] == ESC)
          state = ST_ESC_RCV;
      }
      break;

      case ST_ESC_RCV:
      {
        if (buf[size_buf] != ESC_FLAG & buf[size_buf] != ESC_ESC)
          error = true;

        state = ST_BCC_OK;
      }
      break;
      }
    }

    if (!error)
    {
      //Byte destuffing, returns size of buffer
      size_buffer = byteDeStuffing(buffer, buf, size_buf);

      //Check BCC2
      if (checkBCC2(buffer, size_buffer))
      {
        r = (r + 1) % 2;
        answer = C_RR(r);
      }
      else //Free memory allocated for buffer
        free(buffer);
    }

    //Send acknowlegment
    write(fd, &answer, 1);
    size_buf = DATA_START_INDEX;
    state = ST_D;
  }

  //Free read buf
  free(buf);

  return size_buffer - 1;
}

DataStruct createMessage(unsigned int sequenceNumber, char *buffer, int length)
{

  DataStruct data;
  data.flag = FLAG;
  data.fieldA=A_CE_AR;
  data.fieldC = C(sequenceNumber);
  data.fieldBCC1 = data.fieldA ^ data.fieldC;

  data.fieldBCC2 = (unsigned char *)malloc(sizeof(unsigned char));
  
  //Algortimo de calculo de BCC2 e 
  //I_0=D0//I i+1= I i ^data i

  data.fieldBCC2[0] = buffer[0];

  for (int i = 0; i < length; i++)
  {
    data.fieldBCC2[0] ^= buffer[i];
  }

  //Max size is 2 * length because byte stuffing doubles size
  data.fieldD = (unsigned char *)malloc(sizeof(unsigned char)*length*2);

  data.bcc2StufSize = BCC2Stufying(data.fieldBCC2);
  data.dataStufSize = dataStuffing(buffer, length, data.fieldD);

  return data;
}

unsigned int BCC2Stufying(unsigned char *BCC2)
{
  unsigned int size = 1;

  if (BCC2[0] == FLAG)
  {
    //Vai ser necessario realocar memoria
    BCC2=realloc(BCC2,2*sizeof(unsigned char));
    BCC2[0] = ESC;
    BCC2[1] = ESC_FLAG;
    size = 2;
  }

  else if (BCC2[0] == ESC)
  {
    //Vai ser necessario realocar memoria
    BCC2=realloc(BCC2,2*sizeof(unsigned char));

    BCC2[0] = ESC;
    BCC2[1] = ESC_ESC;
    size = 2;
  }

  return size;
}


unsigned int dataStuffing(unsigned char *data, int length, unsigned char *fieldD)
{

  unsigned int pos = 0;

  for (int i = 0; i < length; i++)
  {
    if (data[i] == FLAG)
     {
      fieldD[i] = ESC;
      pos++;
      fieldD[i + pos] = ESC_FLAG;
    }

    else if (data[i] == ESC)
    {
      fieldD[i] = ESC;
      pos++;
      fieldD[i + pos] = ESC_ESC;

    }
    else{
      
      fieldD[i+pos]=data[i];
    }
  }

  length += pos;

  return length;
}

int llclose(int fd, int flag)
{
  int read_bloc_ret = READ_FAIL;
  n_tries = MAX_RETR;

  
  //Transmitter side
  if (flag == FLAG_LL_CLOSE_TRANSMITTER_DISC)
  {    
    if (signal(SIGALRM, alarm_handler_disc_signal) < 0)
    {
      perror("Erro a instalar o handler no LL_CLOSE, no receiver");
    }

    if (sendBlock(flag,fd) != WRITE_SUCCESS)
    {
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_DISC\n");
      return -1;
    }
    
    type_handling=HANDLING_CLOSE_EMISSOR;
    
    alarm(TIMEOUT);
     
    while (read_bloc_ret == READ_FAIL)
    {
      read_bloc_ret = readBlock(FLAG_LL_CLOSE_TRANSMITTER, fd);
    }
    
    if (read_bloc_ret == READ_FAIL)
    {
      printf("O llclose no transmitter dei timeout sem resposta valida\n");
      return -1;
    }

    
    if (signal(SIGALRM, SIG_IGN) == SIG_ERR)
    {
      perror("Error in ignoring SIG ALARM handler");
    }

    if (sendBlock(FLAG_LL_CLOSE_TRANSMITTER_UA,fd) != WRITE_SUCCESS)
    {
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_UA\n");
      return -1;
    }
  }

  //Receiver block
  else if (flag == FLAG_LL_CLOSE_RECEIVER_DISC)
  {

    printf("Estou aqui\n");

    if (signal(SIGALRM, alarm_handler_disc_signal) < 0)
    {
      perror("Erro a instalar o handler no LL_CLOSE, no receiver");
    }

    if (readBlock(flag, fd) != READ_SUCCESS)
    {
      printf("Erro a receber FLAG_LL_CLOSE_TRANSMITTER_DISC\n");
      return READ_FAIL;
    }

    if (sendBlock(flag, fd) != WRITE_SUCCESS)
    {
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_DISC\n");
      return -1;
    }
    type_handling=HANDLING_CLOSE_RECETOR;

    alarm(TIMEOUT);

    while (read_bloc_ret == READ_FAIL)
    {
      read_bloc_ret = readBlock(FLAG_LL_CLOSE_RECEIVER_UA,fd);
    }

    if (read_bloc_ret == READ_FAIL)
    {
      printf("O llclose no transmitter dei timeout sem resposta valida\n");
      return -1;
    }
  }
  else if(flag==FLAG_HANDLER_CALL){
    
  }
  else
  {
    printf("ERRO EM LLCLOSE");
    return OTHER_ERROR;
  }

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  if (close(fd) != 0)
  {
    perror("Failled to close file");
  }

  printf("LLCLOSE DONE\n");

  return LL_CLOSE_SUCESS;
}
