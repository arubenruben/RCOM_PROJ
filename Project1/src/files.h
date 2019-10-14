#pragma once

/**
* Sends file via application layer
* @param {name of file do send} fileName
* @return {...} number of bytes written
*/
int sendFile(char *fileName);

/**
* Receives file via application layer
* @param {name of file do receive} fileName
* @return {...} 0 on success, -1 otherwise
*/
int receiveFile(char *fileName);

/**
* Returns number of bytes of a file
* @param {filepointer} fp
* @return {...} number of bytes on success, -1 otherwise
*/
int fileSize(FILE *fp);