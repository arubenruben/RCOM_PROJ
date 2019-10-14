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
    unsigned int bcc2StufSize;
    unsigned int dataStufSize;
    size_t size_of_data_frame;
} DataStruct;

/**
* ...description...
* @param {...} porta
* @param {...} flag
* @return {...} 
*/
int llopen(int port_number, int flag);

/**
* ...description...
* @param {file descriptor} fd
* @param {...} buffer
* @param {...} length
* @return {...} 
*/
int llwrite(int fd, unsigned char * buffer, int length);

/**
* ...description...
* @param {file descriptor} fd
* @param {...} buffer
* @return {...} 
*/
int llread(int fd, unsigned char * buffer);

/**
* ...description...
* @param {file descriptor} fd
* @param {...} flag
* @return {...} 
*/
int llclose(int fd, int flag);


