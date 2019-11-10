#pragma once

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE 512
#define PORT_COMMANDS 21
#define PORT_DATA 20


int ftp_login(const int socket_control,const char * username,const char *password);
int ftp_write(const int socket_fd,const char *msg);
int ftp_read(const int socket_fd,int* code_returned,char * string_returned,const int size_of_string_returned_array);
