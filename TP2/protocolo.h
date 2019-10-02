#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "flags.h"

/**
* ...description...
* @param {...} porta
* @param {...} flag
*/
int llopen(int fd, int flag);

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
int llclose(int fd);


/**
* ...description...
* @param {...} porta
* @param {...} buf
* @param {...} n_bytes
*/
int sendBlock(int flag);

/**
* ...description...
* @param {...} porta
* @param {...} buf
* @param {...} n_bytes
*/
int readBlock(int flag);

/**
* ...description...
* @param {...} signo
*/
void alarm_handler(int signo);
