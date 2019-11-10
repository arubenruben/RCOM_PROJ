#include "ftp.h"

//  Private functions
int ftp_password(const int socket_control,const char *password);
int ftp_user(const int socket_control,const char * username);

//DELETE RETURN CODE FROM FTP_rEAD ?-----------------------------------------------
int ftp_retr(const int control_socket_fd, const char *filename) {
    int return_code;
    
    //  Checks control_socket_fd parameter
    if(control_socket_fd < 0) {
        fprintf(stderr,"Invalid file descriptor for control socket in ftp_retr\n");
        return -1;
    }

    //  Checks filename parameter
    if(filename == NULL) {
        fprintf(stderr,"Invalid filename in ftp_retr\n");
        return -1;
    }

    //  Writes the filename to the socket
    if(!(ftp_write(control_socket_fd, filename) > 0)) {
        fprintf(stderr,"Error in writing the filename in ftp_retr\n");
        return -1;
    }

    //  Retrieves information from the socket
    if(!(ftp_read(control_socket_fd,  &return_code, filename, strlen(filename)) > 0)) {
        fprintf(stderr,"Error in retrieving the filename in ftp_retr\n");
        return -1;
    }
    return 0;
}

int ftp_cwd(const int control_socket_fd, const char *path) {
   int return_code;
    
    //  Checks control_socket_fd parameter
    if(control_socket_fd < 0) {
        fprintf(stderr,"Invalid file descriptor for control socket in ftp_retr\n");
        return -1;
    }

    //  Checks path parameter
    if(path == NULL) {
        fprintf(stderr,"Invalid filename in ftp_retr\n");
        return -1;
    }

    //  Writes the path to the socket
    if(!(ftp_write(control_socket_fd, path) > 0)) {
        fprintf(stderr,"Error in writing the path to cwd in ftp_retr\n");
        return -1;
    }

    //  Retrieves information from the socket
    if(!(ftp_read(control_socket_fd,  &return_code, path, strlen(path)) > 0)) {
        fprintf(stderr,"Error in sending path to change directory in ftp_retr\n");
        return -1;
    }
    return 0;
}

int ftp_write(const int socket_fd, const char *msg) {

    size_t n_bytes = -1;

    if(msg == NULL) {
        fprintf(stderr, "Invalid reference in ftp_write\n");
        return -1;
    }

    if((n_bytes = write(socket_fd,msg,strlen(msg))) <= 0) {
        fprintf(stderr,"Error in writing user command\n");
        return -1;
    }

    //Is necessary a \n at the end
    write(socket_fd,"\n", 1);

    fprintf(stdout,">Sent:%ld Byte\n", n_bytes);

    return n_bytes;    
}

int ftp_read(const int socket_fd, int* code_returned, 
                char * string_returned, const int size_of_string_returned_array) {

    if(string_returned == NULL || code_returned == NULL){
        fprintf(stderr,"Invalid reference in ftp_Read\n");
        return -1;
    }
    
    char read_str[2*MAX_BUFFER_SIZE];
    //3digit code + \0
    char code_str[4];
    
    //Ensure every info on string is 0
    memset(read_str, 0, sizeof(read_str));
    
    //Ensure everything is a 0 in this array
    memset(code_str, 0, sizeof(code_str));
    
    size_t n_bytes_read = -1;

    if((n_bytes_read = read(socket_fd,read_str,sizeof(read_str))) <= 0){
        perror("None was read:");
        return -1;
    }

    //Copy the first 3 digits of the message
    strncpy(code_str, read_str, 3);

    *code_returned = atoi(code_str);
    
    //Copy the reply text (read_str+3 DISCARD the 3 digit code)?? +3?
    strncpy(string_returned, read_str, size_of_string_returned_array);

    fprintf(stdout,"Read %ld bytes\n",n_bytes_read);
    fprintf(stdout,"Code %d \n",*code_returned);
    fprintf(stdout,"Msg %s\n",string_returned);
    
    //Parse the code returned
    //According with:https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
    if(*code_returned >= 400) {
        fprintf(stderr,"Code returned is an error");
        return -1;
    }

    else if(*code_returned < 400) {
        return n_bytes_read;
    }

    else {
        fprintf(stderr,"ELSE EXIT\n");
        exit(-1);
    }

    FILE *F = fdopen(socket_fd,"r");
    fflush(F);

    return 0;
}

int ftp_passive_mode(const int socket_control,char *ip,int* port){

    if (ip==NULL||port==NULL){
        fprintf(stderr,"Invalid Refs in passive_mode\n");
        return -1;
    }

    char *cmd="PASV\n";
    int code_returned=-1;
    char str_msg[2*MAX_BUFFER_SIZE];

    if(ftp_write(socket_control,cmd)<0){
        fprintf(stderr,"FTP_WRITE_PASV_ERROR\n");
        return -1;
    }

    if(ftp_read(socket_control,&code_returned,str_msg,sizeof(str_msg))<0){
        fprintf(stderr,"Error receiving the PASV reply\n");
        return -1;
    }
    int ipPart1,ipPart2,ipPart3,ipPart4;
    int portPart1,portPart2;

    fprintf(stdout,"A mensagem antes de dar parsing:%s\n",str_msg);
    
    if((sscanf(str_msg,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).",&ipPart1,&ipPart2,&ipPart3,&ipPart4,&portPart1,&portPart2)<0)){
        fprintf(stderr,"Number of parameters red dont match in ftp_passive_mode\n");
        return -1;
    }

    if ((sprintf(ip,"%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4))<0) {
        fprintf(stderr,"Number of arguments printed out doenst match\n");
		return -1;
	}

    fprintf(stdout,"Podiamos usar um swift esquerdo aqui\n");
    *port=portPart1*256+portPart2;

    fprintf(stdout,"IP:%s\n",ip);
    fprintf(stdout,"Port:%d\n",*port);

    return 0;
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

//  Private functions (declared in top of ftp.c)
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

    sprintf(cmd,"USER %s\n",username);

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