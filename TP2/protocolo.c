#include "protocolo.h"


#define MAX_RETR 3
#define TIMEOUT 3

static struct termios oldtio;
static int n_tries = MAX_RETR;

void alarm_handler(int signo);
int sendBlock(const int flag, const int fd);
int readBlock(const int flag, const int fd);

DataStruct createMessage(unsigned int sequenceNumber, char *buffer, int length);
unsigned int BBC2Stufying (unsigned char *BBC2);
unsigned int dataStuffing (unsigned char *data, int length, unsigned char *fieldD);


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

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 1 char received */

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

void alarm_handler(int signo)
{

  if (signo != SIGALRM)
  {
    printf("Handler nao executado\n");
    return;
  }
  if (n_tries > 0)
  {
    n_tries--;
  }

  return;
}

int checkBCC2(unsigned char *buffer, unsigned int size){
  unsigned char bcc = buffer[size - 2];
  unsigned char bcc_check = 0;

  for(int i = 4; i < (size - 2); i++){
    bcc_check ^= buffer[size];
  }

  return bcc == bcc_check;
}

int byteDeStuffing(unsigned char *dest, unsigned char *orig, unsigned int size_orig){
  int size_dest = 0;

  dest = (unsigned char*) malloc(size_orig * sizeof(unsigned char*));
  if(dest == NULL){
    perror("Failled to allocate memory");
    exit(NO_MEM);
  }

  for(int i = DATA_START_INDEX; i < (size_orig - 1); i++){
    if(orig[i] == ESC){
      i++;
      if(orig[i] == ESC_FLAG)
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

int sendBlock(const int flag, const int fd)
{

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

  else if (flag == FLAG_LL_CLOSE_TRANSMITTER_DISC|| flag==FLAG_LL_CLOSE_TRANSMITTER_UA)
  {

    buf[FLAG_INDEX_BEGIN]=FLAG;

    buf[A_INDEX]=A_CE_AR;

    if(FLAG_LL_CLOSE_RECEIVER_DISC){
      buf[C_INDEX]=C_DISC;

    }else if(FLAG_LL_CLOSE_RECEIVER_UA){
      buf[C_INDEX]=C_UA;
    }

    buf[BCC_INDEX]=buf[A_INDEX]^buf[C_INDEX];

    buf[FLAG_INDEX_END]=FLAG;

    if(write(fd,buf,BUF_SIZE)!=BUF_SIZE){
      perror("Erro no write do FLAG_LL_CLOSE_TRANSMITTER_DISC:");
      return WRITE_FAIL;
    }

  }

   else if (flag == FLAG_LL_CLOSE_RECEIVER_DISC || flag==FLAG_LL_CLOSE_RECEIVER_UA)
  {

    buf[FLAG_INDEX_BEGIN]=FLAG;

    buf[A_INDEX]=A_CR_AE;

    if(FLAG_LL_CLOSE_RECEIVER_DISC){
      buf[C_INDEX]=C_DISC;

    }else if(FLAG_LL_CLOSE_RECEIVER_UA){
      buf[C_INDEX]=C_UA;
    }

    buf[BCC_INDEX]=buf[A_INDEX]^buf[C_INDEX];

    buf[FLAG_INDEX_END]=FLAG;

    if(write(fd,buf,BUF_SIZE)!=BUF_SIZE){
      perror("Erro no write do FLAG_LL_CLOSE_TRANSMITTER_DISC:");
      return WRITE_FAIL;
    }

  }
  else
  {

    printf("SEND BLOCK not implemented\n");
    return WRITE_FAIL;
  }

   return WRITE_SUCCESS;
}

int readBlock(const int flag, const int fd)
{

  unsigned char leitura;
  unsigned int size = 0, state = ST_START;

  if (flag == FLAG_LL_OPEN_RECEIVER || flag == FLAG_LL_OPEN_TRANSMITTER||flag==FLAG_LL_CLOSE_TRANSMITTER_DISC||flag==FLAG_LL_CLOSE_TRANSMITTER_UA||flag==FLAG_LL_CLOSE_RECEIVER_DISC||flag==FLAG_LL_CLOSE_RECEIVER_UA)
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

          if (flag == FLAG_LL_OPEN_TRANSMITTER||flag==FLAG_LL_CLOSE_RECEIVER_UA)
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

          if (flag == FLAG_LL_CLOSE_RECEIVER_DISC||flag==FLAG_LL_CLOSE_TRANSMITTER_DISC)
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
        else if(leitura==(A_CE_AR^C_DISC)&&flag==FLAG_LL_CLOSE_RECEIVER_DISC){
          state=ST_BCC_OK;
        }
        else if(leitura==(A_CR_AE^C_DISC)&&flag==FLAG_LL_CLOSE_TRANSMITTER_DISC){
          state=ST_BCC_OK;
        }

        else if(leitura==(A_CE_AR^C_UA)&&flag==FLAG_LL_CLOSE_RECEIVER_UA){
          state=ST_BCC_OK;
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

  if (fd == UNKNOWN_PORT)
  {
    printf("Unknown port, must be either 0 or 1\n");
    exit(UNKNOWN_PORT);
  }

  //Transmitter
  if (flag == FLAG_LL_OPEN_TRANSMITTER)
  {
    int ret_read_block = READ_FAIL;

    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
      perror("Error instaling SIG ALARM handler\n");
      return -1;
    }

    if (sendBlock(FLAG_LL_OPEN_TRANSMITTER, fd) != WRITE_SUCCESS)
    {
      printf("Error in sendSet function\n");
    }

    //Set alarm
    alarm(TIMEOUT);

    while (n_tries > 0&&ret_read_block==READ_FAIL)
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
  return fd;
}


int llwrite(int fd, char * buffer, int length)  {

  static unsigned int sequenceNumber = 0;
  int num_bytes = 0;
  int dataStufSize = length;
  int bcc2StufSize = 1;
  unsigned char answer = C_REJ((sequenceNumber + 1) % 2);
  unsigned char correctAnswer = C_RR((sequenceNumber + 1) % 2);

  DataStruct data = createMessage(sequenceNumber, buffer, length);

  if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
    perror("Error in ignoring SIG ALARM handler");
  }

  //Timeout Cycle
  while(n_tries && answer != correctAnswer){

    if((num_bytes = write(fd, &data, 4 * sizeof(unsigned char))) != 4)
      return -1;

    //REJ Cycle
    while(answer != correctAnswer) {

      if((num_bytes = write(fd, &data.fieldD, dataStufSize*sizeof(unsigned char))) != dataStufSize)
        return -1;

      if((num_bytes = write(fd, &data.fieldBCC2, sizeof(unsigned char))) != 1)
        return -1;

      if((num_bytes = write(fd, &data.flag, sizeof(unsigned char))) != 1)
        return -1;

      alarm(TIMEOUT);

      read(fd, &answer, 1);

      signal(SIGALRM, SIG_IGN);
    }
  }

  free(data.fieldD);
  free(data.fieldBCC2);

  sequenceNumber = (sequenceNumber + 1) % 2;

  return (dataStufSize + 5);
}

int llread(int fd, char *buffer){
  static unsigned int r = 0;

  unsigned int size_buf = 0, state = ST_START, max_size = MAX_BUF;
  bool error = false;
  unsigned char *buf = NULL;
  unsigned char answer = C_REJ(r);
  int size_buffer = 0;

  if(buffer == NULL || fd < 0){
    printf("Passei parametros invalidos a llread\n");
    return INVALID_PARAMS;

  }

  buf = (unsigned char*) malloc(max_size * sizeof(unsigned char));

  if(buf == NULL){
    perror("Failled to allocate memory");
    exit(NO_MEM);
  }

  //While data is rejected be cause of errors go to state machine
  while(answer == C_REJ(r)){

    //State machine
    for ( ; state != ST_STOP; size_buf++){
      //Check size
      if(size_buf > max_size){
        max_size *= 2;
        buf = (unsigned char*) realloc((void*)buf, max_size * sizeof(unsigned char));

        if(buf == NULL){
          perror("Failled to allocate memory");
          exit(NO_MEM);
        }

      }

      //Read byte
      if (!read(fd, &buf[size_buf], 1)){
        free(buf);
        perror("Failled to read");
        return READ_FAIL;
      }

      //Go through state machine
      switch (state) {

        case ST_START:{

          if(buf[size_buf] == FLAG)
            state = ST_FLAG_RCV;
          else size_buf=START_INDEX;
        }

        break;

        case ST_FLAG_RCV:{

          if(buf[size_buf] == A_CE_AR)
            state = ST_A_RCV;

          else if(buf[size_buf] != FLAG){
            state = ST_START;
            size_buf =START_INDEX;
          }

          else size_buf--;
        }

        break;

        case ST_A_RCV:{

          if(buf[size_buf] == C(r))
            state = ST_C_RCV;

          else if(buf[size_buf] == FLAG){
            state = ST_FLAG_RCV;
            size_buf -= 2;
          }

          else{
            state = ST_START;
            size_buf = START_INDEX;
          }

        }
        break;

        case ST_C_RCV:{

          if(buf[size_buf] == (A_CE_AR^C(r)) )
            state = ST_BCC_OK;

          else if(buf[size_buf] == FLAG){
            state = ST_FLAG_RCV;
            size_buf -= 3;
          }
          else{
            state = ST_START;
            size_buf =START_INDEX;
          }
        }
        break;

        case ST_D:{

          if(buf[size_buf] == FLAG)
            state = ST_STOP;
          else if(buf[size_buf] == ESC)
            state = ST_ESC_RCV;
        }
        break;

        case ST_ESC_RCV:{
          if(buf[size_buf] != ESC_FLAG & buf[size_buf] != ESC_ESC)
            error = true;

          state = ST_BCC_OK;

        }
          break;
      }
    }


    if(!error){
      //Byte destuffing, returns size of buffer
      size_buffer = byteDeStuffing(buffer, buf, size_buf);

      //Check BCC2
      if(checkBCC2(buffer, size_buffer)){
        r = (r+1) % 2;
        answer = C_RR(r);
      }
      else  //Free memory allocated for buffer
        free(buffer);
    }

    //Send acknowlegment
    write(fd, &answer, 1);
    size_buf = DATA_START_INDEX;
    state = ST_D;
  }

  //Free read buf
  free(buf);

  return size_buffer-1;
}

DataStruct createMessage(unsigned int sequenceNumber, char* buffer, int length) {

  DataStruct data;
  data.flag = FLAG;
  data.fieldC = C(sequenceNumber);
  data.fieldBCC1 = data.fieldA ^ data.fieldC;
  data.fieldBCC2 = (unsigned char*) malloc(2 * sizeof(unsigned char));
  data.fieldBCC2[0] = buffer[0];

  for(int i = 0; i < length; i++) {
    data.fieldBCC2[0] ^=  data.fieldD[i];
  }

  //Max size is 2 * length because byte stuffing doubles size
  data.fieldD = (unsigned char *) malloc(2 * length);

  data.bcc2StufSize = BBC2Stufying(data.fieldBCC2);
  data.dataStufSize = dataStuffing(buffer, length, data.fieldD);

  return data;
}

unsigned int BBC2Stufying (unsigned char *BBC2) {
  unsigned int size = 1;

  if(*BBC2 == FLAG) {
    BBC2[0] = ESC;
    BBC2[1] = ESC_FLAG;
    size = 2;
  }

  else if(*BBC2 == ESC) {
    BBC2[0] = ESC;
    BBC2[1] = ESC_ESC;
    size = 2;
  }

  return size;
}

unsigned int dataStuffing (unsigned char* data, int length, unsigned char *fieldD) {

  unsigned int pos = 0;

  for (int i = 0; i < length; i++) {

    data[i + pos] = data[i];

    if(data[i] == FLAG) {
      fieldD[i] = ESC;
      pos++;
      fieldD[i+pos] = ESC_FLAG;
    }

    else if(data[i] == ESC) {
      fieldD[i] = ESC;
      pos++;
      fieldD[i+pos] = ESC_ESC;
    }
  }

  length += pos;

  return length;
}

int llclose(int fd, int flag)
{
  int read_bloc_ret=READ_FAIL;
  n_tries=MAX_RETR;

  //Transmitter side
  if (flag == FLAG_LL_CLOSE_TRANSMITTER_DISC)
  {

    if (sendBlock(flag, FLAG_LL_CLOSE_TRANSMITTER_DISC) != WRITE_SUCCESS)
    {
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_DISC\n");
      return -1;
    }

    alarm(TIMEOUT);


    while(n_tries>0&&read_bloc_ret==READ_FAIL){

      read_bloc_ret=readBlock(flag,FLAG_LL_CLOSE_TRANSMITTER_DISC);

    }

    if(read_bloc_ret==READ_FAIL){
      printf("O llclose no transmitter dei timeout sem resposta valida\n");
      return -1;
    }

    if (signal(SIGALRM, SIG_IGN) == SIG_ERR)
    {
      perror("Error in ignoring SIG ALARM handler");
    }


    if (sendBlock(flag, FLAG_LL_CLOSE_TRANSMITTER_UA) != WRITE_SUCCESS)
    {
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_UA\n");
      return -1;
    }

  }

  //Receiver block
  else if (flag == FLAG_LL_CLOSE_RECEIVER_DISC)
  {

    if(signal(SIGALRM,alarm_handler)<0){
      perror("Erro a instalar o handler no LL_CLOSE, no receiver");
    }


    if(readBlock(flag,FLAG_LL_CLOSE_RECEIVER_DISC)!=READ_SUCCESS){
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_UA\n");
      return -1;

    }

    if (sendBlock(flag, FLAG_LL_CLOSE_RECEIVER_DISC) != WRITE_SUCCESS)
    {
      printf("Erro a enviar FLAG_LL_CLOSE_TRANSMITTER_DISC\n");
      return -1;
    }

    alarm(TIMEOUT);


    while(n_tries>0&&read_bloc_ret==READ_FAIL){

      read_bloc_ret=readBlock(flag,FLAG_LL_CLOSE_RECEIVER_UA);

    }

    if(read_bloc_ret==READ_FAIL){
      printf("O llclose no transmitter dei timeout sem resposta valida\n");
      return -1;
    }
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

  return 0;
}
