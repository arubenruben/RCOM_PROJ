#include "protocolo.h"
#include "application.h"
#include "files.h"

int sendFile(char *fileName) {
    FILE* file = fopen(fileName, "rb");
    //int fd = llopen

    //send control package - START

    // while reads file sendDataPackage

    // closes (fclose, sendcontrol -END, llclose)
}

int receiveFile() {
    //int fd = llopen
}

int fileSize() {
    //while fseek != null counter++..

}