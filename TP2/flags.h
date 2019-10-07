#pragma once

//Port settings
#define BAUDRATE B38400
#define MODEMDEVICE_0 "/dev/ttyS0"
#define MODEMDEVICE_1 "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define UNKNOWN_PORT -1
#define OTHER_ERROR -2


#define FLAG_LL_OPEN_TRANSMITTER 1
#define FLAG_LL_OPEN_RECEIVER 2

#define FLAG_LL_CLOSE_TRANSMITTER_DISC 1
#define FLAG_LL_CLOSE_TRANSMITTER_UA 2
#define FLAG_LL_CLOSE_RECEIVER_DISC 3
#define FLAG_LL_CLOSE_RECEIVER_UA 4

//5 Byte message
#define FLAG_INDEX_BEGIN 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define FLAG_INDEX_END 4

//Flags values
#define FLAG 0x7E
#define A_CE_RR 0x03
#define A_CR_RE 0x01
#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B

//Other variables
#define INVALID_PARAMS -1
#define READ_SUCCESS 0
#define READ_FAIL 1
#define WRITE_SUCCESS 0
#define WRITE_FAIL 1
#define BUF_SIZE 5
#define MAX_BUF 255

//State machine flags
#define ST_START 0
#define ST_FLAG_RCV 1
#define ST_A_RCV 2
#define ST_C_RCV 3
#define ST_BCC_OK 4
#define ST_STOP 5
