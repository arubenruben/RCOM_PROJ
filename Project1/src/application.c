#include "protocolo.h"
#include "application.h"
#include "files.h"

int sendDataBlock(int fd, uint sequenceNumber, uchar *buffer, uint length) {
    AppDataStruct data;

    // Create AppDataStruct
    data.fieldC = Data;
    data.fieldN = sequenceNumber;
    data.fieldL2 = length / 256;
    data.fieldL1 = length % 256;
    memcpy(data.fieldP, buffer, length);

    // Write AppDataStruct to Data Link layer
    if(llwrite(fd, (uchar*)&data, sizeof(AppDataStruct)) < 0) {
        printf("Error while writing data block to Data Link layer!\n");
        return -1;
    }

    return 0;
}

int receiveDataBlock(int fd, uint *sequenceNumber, uchar *buffer) {
    AppDataStruct data;
    uint length;

    if(buffer == NULL) {
        printf("Received buffer is NULL!\n");
        return -1;
    }

    // Read block from Data Link layer
    if(llread(fd, (uchar*)&data) < 0) {
        printf("Error while reading data block from Data Link layer!\n");
        return -1;
    }

    // Checks if block is a data block
    if (data.fieldC != Data) {
        printf("Error: Received block is not a data block!\n");
        return -1;
    }

    *sequenceNumber = data.fieldN;
    length = data.fieldL2 * 256 + data.fieldL1;

    // Does a copy of data from block received
    memcpy(buffer, data.fieldP, length);

    return length;
}

int sendControlBlock(int fd, int fieldC, uint fileSize, char *fileName) {
    AppControlStruct control;

    // Converts fileSize to char* to send via TLV struct
    char fileSizeString[64];
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
    control.fileName.value[strlen(fileName)] = '\0';
    control.fileName.length++;

   // Write AppControlStruct to Data Link layer
    if(llwrite(fd, (uchar*)&control, sizeof(AppControlStruct)) < 0) {
        printf("Error while writing fileSize to Data Link layer!\n");
        return -1;
    }

    return 0;
}

int receiveControlBlock(int fd, uint *type , char *fileName) {
    AppControlStruct control;

    if(fileName == NULL) {
        printf("Received fileName is NULL\n");
        return -1;
    }

    if(llread(fd, (uchar*)&control) < 0) {
        printf("Error while reading control block from Data Link layer!\n");
        return -1;
    }

    // Control block type
    *type = control.fieldC;

    // Checks if block is a control block
    if (control.fieldC != Start && control.fieldC != End) {
        printf("Error: Received block is not a control block!\n");
        return -1;
    }

    // FileName
    memcpy(fileName, control.fileName.value, control.fileName.length);
    
    // Returns FileSize
    return atoi((char*)control.fileSize.value);
}

int main(int argc, char* argv[]) {

    if(argc != 3) {
        printf("Usage:\tex: \t./a.out \t\"name\".txt \t0\n");
        exit(1);
    }

    char* filename = argv[1];
    if(filename == NULL){
        printf("Referencia para o filename invalida\n");
    }

    switch (atoi(argv[2])) 
    {
        case Receiver:
            if(receiveFile() != 0) {
                printf("Error in receiveFile!\n");
                return -1;
            }
            break;
        
        case Sender:
            if(sendFile(filename) < 0) {
                printf("Error in sendFile!\n");
                return -1;
            }
            break;
        
        default:
            printf("Error in second argument(0/1)!\n");
            printf("Exiting...\n");
            return -1;
    }
}