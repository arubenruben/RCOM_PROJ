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

typedef enum {
    Data = 1, 
    Start = 2, 
    End = 3
} ControlField; 

typedef struct {
    ControlField fieldC;
    uint fieldN;
    uint fieldL2;
    uint fieldL1;
    unsigned char* fieldP;
} AppDataStruct;

typedef struct {
    uint fieldC;
    uint fieldT;
    uint fieldL;
    uint fieldV;
} AppControlStruct;



/**
* ...description...
* @param {...} porta
* @param {...} flag
*/
int sendDataBlock(int fd, uint sequenceNumber, char* buffer, uint length);

/**
* ...description...
* @param {...} fd
* @param {...} flag
*/
int receiveDataBlock(int fd, int* N, char** buf, int* length);

/**
* ...description...
* @param {...} porta
* @param {...} flag
*/
int sendControlBlock(int fd, int fieldC, char* fileSize, char* fileName);

/**
* ...description...
* @param {...} porta
* @param {...} flag
*/
int receiveControlBlock(int fd, int* controlPackageType, int* fileLength, char** fileName);
