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

    // Write block to Data Link layer
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

int sendControlBlock(int fd, int fieldC, char *fileSize, char *fileName) {
    AppControlStruct control;
    
    // Alocates memory value in TLV message
    control.tlv[0].value = (char*)malloc(sizeof(char) * strlen(fileSize));
    control.tlv[1].value = (char*)malloc(sizeof(char) * strlen(fileName));

    // Create AppControlStruct
    control.fieldC = fieldC;
    control.tlv[0].type = FileSize;
    control.tlv[0].length = strlen(fileSize);
    for (uint i = 0; i < strlen(fileSize); i++)
		control.tlv[0].value[i] = fileSize[i];

    control.tlv[1].type = FileName;
    control.tlv[1].length = strlen(fileName);
    for (uint i = 0; i < strlen(fileName); i++)
		control.tlv[1].value[i] = fileName[i];
    
    control.length = strlen(fileSize) + strlen(fileSize) + 5;

   // Write FileSize
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
    char *size = (char *)malloc(sizeof(char) * control.tlv[0].length);
    memcpy(size, control.tlv[0].value, control.tlv[0].length);

    // FileName
    memcpy(fileName, control.tlv[1].value, control.tlv[1].length);

    // Returns FileSize
    return atoi(size);
}