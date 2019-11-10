

#include "ftp.h"
#include <stdlib.h>



int ftp_user(const int socket_control,const char * username);
int ftp_password(const int socket_control,const char *password);



int ftp_user(const int socket_control,const char * username){

    if(username==NULL){
        fprintf(stderr,"Ref invalida em ftp_user\n");
        return -1;
    }
    
    //this is the command FTP to prepare ther server to receive an authentication
    char cmd[2*MAX_BUFFER_SIZE];
    int code=-1;
    char reply[MAX_BUFFER_SIZE];
    //Ensures 0 values is set
    memset(reply,0,sizeof(reply));

    sprintf(cmd,"USER %s\r\n",username);

    int n_bytes_lidos;
    
    //SEND THE USER COMMAND + USERNAME
    if(ftp_write(socket_control,cmd)<0){
        perror("Error sending username:");
        exit(-1);
    }
    //READ THE REPLY TO USER COMMAND
    if((n_bytes_lidos=ftp_read(socket_control,&code,reply,sizeof(reply)))<0){
        fprintf(stderr,"Nada lido no user reply\n");
        return -1;
    }

    return 0;
}

int ftp_write(const int socket_fd,const char *msg){

    size_t n_bytes=-1;

    if(msg==NULL){
        fprintf(stderr,"Ref invalida em ftp_write\n");
        return -1;
    }
    if((n_bytes=write(socket_fd,msg,strlen(msg)))<=0){
        fprintf(stderr,"Error in writing user command\n");
        return -1;
    }
    //Is necessary a \n at the end
    write(socket_fd,"\n",1);
    
    fprintf(stdout,">Sent:%ld Byte\n",n_bytes);
    

    return n_bytes;    
}


int ftp_read(const int socket_fd,int* code_returned,char * string_returned,const int size_of_string_returned_array){

    if(string_returned==NULL||code_returned==NULL){
        fprintf(stderr,"Invalid reference in ftp_Read\n");
        return -1;
    }
    
    char read_str[2*MAX_BUFFER_SIZE];
    //Ensure every info on string is 0
    memset(read_str,0,sizeof(read_str));

    //3digit code + \0
    char code_str[4];
    
    //Ensure everything is a 0 in this array
    memset(code_str,0,sizeof(code_str));
    
    size_t n_bytes_read=-1;


    if((n_bytes_read=read(socket_fd,read_str,sizeof(read_str)))<=0){
        perror("None was read:");
        return -1;
    }
    //Copy the first 3 digits of the message
    strncpy(code_str,read_str,3);

    *code_returned=atoi(code_str);
    
    //Cpy the reply text (read_str+3 DISCARD the 3 digit code)
    strncpy(string_returned,read_str+3,size_of_string_returned_array);


    fprintf(stdout,"Read %ld bytes\n",n_bytes_read);
    fprintf(stdout,"Code %d \n",*code_returned);
    fprintf(stdout,"Msg %s",string_returned);
    
    fprintf(stdout,"MULTIPLES LINES NEEDS TO BE FIXED\n");

    FILE *F=fdopen(socket_fd,"r");
    fflush(F);


    

    //Parse the code returned
    
    //According with:https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
    if(*code_returned>=400){
        fprintf(stderr,"Code returned is an error");
        return -1;
    }
    else if(*code_returned<400){
        return n_bytes_read;
    }
    else{
        fprintf(stderr,"ELSE EXIT\n");
        exit(-1);
    }

}

int ftp_login(const int socket_control,const char * username,const char *password){

    if(username==NULL||password==NULL){
        fprintf(stderr,"Invalid refs e in FTP_LOGIN\n");
        return -1;
    }

    if(ftp_user(socket_control,username)<0){
        fprintf(stderr,"Erro no FTP_USER\n");
        return -1;
    }

    if(ftp_password(socket_control,password)<0){
        fprintf(stderr,"Erro no FTP_PASSWORD\n");
        return -1;
    }

    return 0;
}

int ftp_password(const int socket_control,const char *password){
    
    if(password==NULL){
        fprintf(stderr,"Ref invalida em ftp_user\n");
        return -1;
    }
    
    //this is the command FTP to prepare ther server to receive an authentication
    char cmd[2*MAX_BUFFER_SIZE];
    int code=-1;
    char reply[MAX_BUFFER_SIZE];
    //Ensures 0 values is set
    memset(reply,0,sizeof(reply));
    
    //Constructs the formated string
    sprintf(cmd,"PASS %s\r\n",password);

    int n_bytes_lidos;
    
    //SEND THE USER COMMAND + password
    if(ftp_write(socket_control,cmd)<0){
        perror("Error sending password:");
        exit(-1);
    }
    //READ THE REPLY TO USER COMMAND
    if((n_bytes_lidos=ftp_read(socket_control,&code,reply,sizeof(reply)))<0){
        fprintf(stderr,"Nada lido no user reply\n");
        return -1;
    }

    return 0;
}