#include "protocolo.h"
#include "application.h"
#include "files.h"

int sendFile(char *fileName) {
    FILE* file = fopen(fileName, "rb");
    if(file == NULL) {
        printf("Could not open %s!\n", fileName);
        return -1;
    }
    
    int fd = llopen(0, FLAG_LL_OPEN_TRANSMITTER);
    if(fd < 0) {
        printf("Error in llopen!\n");
        return -1;
    }

    // send control package - START
    if(sendControlBlock(fd, Start, fileSize(file), fileName) < 0) {
        printf("Error in sendControlBlock!\n");
        return -1;
    }

    uint length, nBytes = 0, sequenceNumber = 0;
    char buffer[MAX_BUF];

    // while reads file sendDataPackage
    while((length = fread(buffer, sizeof(char), MAX_BUF, file)) != EOF) {
        if(sendDataPackage(fd, sequenceNumber%255, buffer, length) != EOF) {
            printf("Error in sendDataPackage!\n");
            return -1;
        }
        sequenceNumber++;
        nBytes += length;
    }

    // closes (fclose, sendcontrol -END, llclose)
    if(fclose(file) != 0) {
        printf("Error while closing file!\n");
        return -1;
    }

    // send control package - END
    if(sendControlBlock(fd, End, fileSize(file), fileName) < 0) {
        printf("Error in sendControlBlock!\n");
        return -1;
    }

    if(llclose(fd, FLAG_LL_CLOSE_TRANSMITTER) != 0) {
        printf("Error in llclose!\n");
        return -1;
    }

    return nBytes;
}

int receiveFile(char *fileName) {
    FILE* file = fopen(fileName, "w");
    if(file == NULL) {
        printf("Could not open/create %s!\n", fileName);
        return -1;
    }


    int fd = llopen(0, FLAG_LL_OPEN_TRANSMITTER);
    if(fd < 0) {
        printf("Error in llopen!\n");
        return -1;
    }

    uint controlType, sequenceNumber = 0;

    // receive control block - START
    if(receiveControlBlock(fd, &controlType, fileName) < 0) {
        printf("Error in sendControlBlock!\n");
        return -1;
        
        if(controlType != Start) {
            printf("controlType value is not START\n");
            return -1;
        }
    }

    char buffer[MAX_BUF];

    // receive data block - DATA
    if(receiveDataBlock(fd, &sequenceNumber, buffer) != 0) {
        printf("Error in receiveDataBlock!\n");
        return -1;
    }
}

int fileSize(FILE *fp) {
    uint counter = 0;

    if(fp == NULL) {
        printf("File pointer is NULL!\n");
        return -1;
    }

    if(fseek(fp, 0, SEEK_END) < 0) {
        printf("Error in fseek!\n");
        return -1;
    }

    if((counter = ftell(fp)) < 0) {
        printf("Error in ftell!\n");
        return -1;
    }

    if(fseek(fp, 0, SEEK_SET) < 0) {
        printf("Error in fseek!\n");
        return -1;
    }

    return counter;
}

int main(int argc, char* argv[]){

    if(argc!=3){
        printf("N de args errado\n");
    }

    char filename[BUF_SIZE];
    //filename copiado dos argumentos
    strcpy(filename,argv[1]);






    



}