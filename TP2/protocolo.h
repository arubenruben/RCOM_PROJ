#pragma once

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



typedef struct {
    unsigned char flag ;
    unsigned char fieldA;
    unsigned char fieldC;
    unsigned char fieldBCC1;
    unsigned char* fieldD;
    unsigned char* fieldBCC2;
    
    //Other important variables
    unsigned int *bcc2StufSize;
    unsigned int *dataStufSize;
} DataStruct;

/**
* ...description...
* @param {...} porta
* @param {...} flag
*/
int llopen(int port_number, int flag);

/**
* ...description...
* @param {...} fd
* @param {...} buffer
* @param {...} length
*/
int llwrite(int fd, char * buffer, int length);

/**
* ...description...
* @param {...} fd
* @param {...} buffer
*/
int llread(int fd, char * buffer);

/**
* ...description...
* @param {...} fd
*/
int llclose(int fd,int flag);


