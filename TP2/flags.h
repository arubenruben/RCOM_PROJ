#pragma once

#define FLAG_LL_OPEN_TRANSMITTER 1
#define FLAG_LL_OPEN_RECEIVER 2

//5 Byte message
#define FLAG_INDEX_BEGIN 0
#define A_INDEX 1
#define C_INDEX 2
#define BCC_INDEX 3
#define FLAG_INDEX_END 4

//Flags values
#define FLAG 0x99
#define A_EM 0x03
#define A_RE 0x01
#define C_SET 0x03
#define C_UA 0x07

//Other variables
#define INVALID_PARAMS -1
#define READ_SUCCESS 0
#define READ_FAIL 1
#define BUF_SIZE 5
#define MAX_BUF 255

//State machine flags
#define ST_START 0
#define ST_FLAG_RCV 1
#define ST_A_RCV 2
#define ST_C_RCV 3
#define ST_BCC_OK 4
#define ST_STOP 5
