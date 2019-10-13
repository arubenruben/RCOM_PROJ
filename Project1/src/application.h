#pragma once

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
    Type type;
    uchar length;
    char *value;
} TLV;

typedef struct {
    ControlField fieldC;
    uchar fieldN;
    uchar fieldL2;
    uchar fieldL1;
    char *fieldP;
    uchar length;
} AppDataStruct;

typedef struct {
    ControlField fieldC;
    uchar length;
    TLV tlv[2];
} AppControlStruct;


/**
* ...description...
* @param {file descriptor} fd
* @param {sequence number (%255)} sequenceNumber
* @param {data} buffer
* @param {data length} length
* @return {...} 0 on success, -1 otherwise
*/
int sendDataBlock(int fd, uint sequenceNumber, char* buffer, uint length);

/**
* ...description...
* @param {file descriptor} fd
* @param {returns sequence number (%255)} sequenceNumber
* @param {returns data block received} buffer
* @return {...} length of block received on success, -1 otherwise
*/
int receiveDataBlock(int fd, int *sequenceNumber, char *buffer);

/**
* ...description...
* @param {file descriptor} fd
* @param {control field(START/END)} fieldC
* @param {TLV first message} fileSize
* @param {TLV second message} fileName
* @return {...} 0 on success, -1 otherwise
*/
int sendControlBlock(int fd, int fieldC, char *fileSize, char *fileName);

/**
* ...description...
* @param {file descriptor} fd
* @param {returns control field(START/END)} type
* @param {returns fileName} fileName
* @return {...} FileSize on success, -1 otherwise
*/
int receiveControlBlock(int fd, uint *type, char *fileName);
