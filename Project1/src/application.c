#include "protocolo.h"
#include "application.h"

int sendDataBlock(int fd, uint sequenceNumber, char *buffer, uint length) {
    AppDataStruct data;

    // Create AppDataStruct
    data.fieldC = Data;
    data.fieldN = sequenceNumber;
    data.fieldL2 = length / 256;
    data.fieldL1 = length % 256;
    memcpy(data.fieldP, buffer, length);
    data.length = length + 4;

    // Write AppDataStruct to Data Link layer
    if(llwrite(fd, (char*)&data, data.length) < 0) {
        printf("Error while writing data block to Data Link layer!\n");
        return -1;
    }

    return 0;
}

int receiveDataBlock(int fd, int *sequenceNumber, char *buffer) {
    AppDataStruct data;
    uint length;

    // Read block from Data Link layer
    if(llread(fd, (char*)&data) < 0) {
        printf("Error while reading data block from Data Link layer!\n");
        return -1;
    }

    // Checks if block is a data block
    if (data.fieldC != Data) {
        printf("Error: Received block is not a data block!\n");
        return -1;
    }

    length = 256 * data.fieldL2 + data.fieldL1;

    *sequenceNumber = data.fieldN;

    // Allocates memory for buffer and does a copy of data from block received
    buffer = (char *)malloc(sizeof(char) * length);
    memcpy(buffer, data.fieldP, length);

    return length;
}

int sendControlBlock(int fd, int fieldC, int fileSize, char *fileName) {
    AppControlStruct control;
    
    // Alocates memory value in TLV message
    control.fileSize.value = (char*)malloc(sizeof(char) * strlen(fileSize));
    control.fileName.value = (char*)malloc(sizeof(char) * strlen(fileName));

    char fileSizeString[15];
    sprintf(fileSizeString, "%d", fileSize);

    // Create AppControlStruct
    control.fieldC = fieldC;
    //FileSize
    control.fileSize.type = FileSize;
    control.fileSize.length = strlen(fileSizeString);
    for (uint i = 0; i < strlen(fileSizeString); i++)
		control.fileSize.value[i] = fileSizeString[i];
    //FileName
    control.fileName.type = FileName;
    control.fileName.length = strlen(fileName);
    for (uint i = 0; i < strlen(fileName); i++)
		control.fileName.value[i] = fileName[i];
    
    control.length = strlen(fileSize) + strlen(fileSize) + 5;

   // Write AppControlStruct to Data Link layer
    if(llwrite(fd, (char*)&control, control.length) < 0) {
        printf("Error while writing fileSize to Data Link layer!\n");
        return -1;
    }
    return 0;
}

int receiveControlBlock(int fd, uint *type , char *fileName) {
    AppControlStruct control;

    // Read block from Data Link layer
    if(llread(fd, (char*)&control) < 0) {
        printf("Error while reading control block from Data Link layer!\n");
        return -1;
    }

    // Checks if block is a control block
    if (control.fieldC != Start && control.fieldC != End) {
        printf("Error: Received block is not a control block!\n");
        return -1;
    }

    // Control block type
    *type = control.fieldC;

    // FileSize
    char *size = (char *)malloc(sizeof(char) * control.fileSize.length);
    memcpy(size, control.fileSize.value, control.fileSize.length);

    // FileName
    memcpy(fileName, control.fileName.value, control.fileName.length);

    // Returns FileSize
    return atoi(size);
}