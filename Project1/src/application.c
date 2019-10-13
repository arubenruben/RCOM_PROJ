#include "application.h"

int sendDataBlock(int fd, uint sequenceNumber, char *buffer, uint length) {
    AppDataStruct data;

    //Create AppDataStruct
    data.fieldC = Data;
    data.fieldN = sequenceNumber;
    data.fieldL2 = length / 256;
    data.fieldL1 = length % 256;
    memcpy(data.fieldP, buffer, length);
    data.length = length + 4;

    //Write block
    if(llwrite(fd, data, data.length) < 0) {
        printf("Error while writing data block to Data Link layer!\n");
        return -1;
    }

    return 0;
}

int sendControlBlock(int fd, int fieldC, char *fileSize, char *fileName) {
    AppControlStruct control;
    
    //Alocates memory value in TLV message
    control.tlv[0].value = (char*)malloc(sizeof(char) * strlen(fileSize));
    control.tlv[1].value = (char*)malloc(sizeof(char) * strlen(fileName));

    //Create AppControlStruct
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

   //Write FileSize
    if(llwrite(fd, control.tlv, control.length) < 0) {
        printf("Error while writing fileSize to Data Link layer!\n");
        return -1;
    }

    return 0;
}