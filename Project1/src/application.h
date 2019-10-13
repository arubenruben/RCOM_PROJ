#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "flags.h"

typedef unsigned int uint;
typedef unsigned char uchar;

typedef enum {
    Data = 1, 
    Start = 2, 
    End = 3
} ControlField; 

typedef enum {
    FileSize = 1, 
    FileName = 2, 
    //Define other values if necessary
} Type; 

typedef struct {
    ControlField fieldC;
    uchar fieldN;
    uchar fieldL2;
    uchar fieldL1;
    char *fieldP;
} AppDataStruct;

typedef struct {
    uchar fieldC;
    uchar length;
    TLV tlv[2];
} AppControlStruct;

typedef struct {
  Type type;
  uchar length;
  char *value;
} TLV;


/**
* ...description...
* @param {file descriptor} fd
* @param {sequence number (%255)} sequenceNumber
* @param {data} buffer
* @param {data length} length
* @return {...} 0 in success, -1 otherwise
*/
int sendDataBlock(int fd, uint sequenceNumber, char* buffer, uint length);

/**
* ...description...
* @param {file descriptor} fd
* @param {...} buf
* @param {...} length
* @return {...} 0 in success, -1 otherwise
*/
int receiveDataBlock(int fd, int* N, char** buf, int* length);

/**
* ...description...
* @param {file descriptor} fd
* @param {control field(START/END)} fieldC
* @param {TLV first message} fileSize
* @param {TLV second message} fileName
* @return {...} 0 in success, -1 otherwise
*/
int sendControlBlock(int fd, int fieldC, char* fileSize, char* fileName);

/**
* ...description...
* @param {...} fd
* @param {...} controlPackageType
* @param {...} fileLength
* @param {...} fileName
* @return {...} 0 in success, -1 otherwise
*/
int receiveControlBlock(int fd, int* controlPackageType, int* fileLength, char** fileName);
