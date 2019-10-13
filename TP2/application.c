#include "application.h"

int sendDataBlock(int fd, uint sequenceNumber, char* buffer, uint length) {
    AppDataStruct data;

    data.fieldC = Data;
    data.fieldN = sequenceNumber;
    data.fieldL2 = ;
    data.fieldL1 = ;
    memcpy(data.fieldP, buffer, length);

    //Write block
    if(llwrite(fd, buffer, length) < 0) {
        printf("Error while writing data block to Data Link layer!\n");
        return -1;
    }

    return 0;
}