#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>

//State Machine to parse input
enum STATES{

    ST_READ_FTP,
    ST_FAIL,
    ST_READ_USER,
    ST_READ_PASSWORD,
    ST_READ_HOST,
    ST_READ_URL_PATH

};

typedef int STATE;


//Sets the maximum value the output of buffer can contain
#define MAX_BUFFER_SIZE 512


int parseInput(const char * input,char * user,char * password,char * host,char * path,char * filename);

void buffers_cleaner(char * user,char * password,char * host,char * url_path){

    memset(user,0,MAX_BUFFER_SIZE);
    memset(password,0,MAX_BUFFER_SIZE);
    memset(host,0,MAX_BUFFER_SIZE);
    memset(url_path,0,MAX_BUFFER_SIZE);

}

//struct hostent DNS_CONVERT(char )



int main(int argc, char * argv[]){


    if(argc!=2){
        fprintf(stderr,"Num args invalid\n");
    }

    if(argv[1]==NULL){
        fprintf(stderr,"Null argv pointer");
    }


    char user[MAX_BUFFER_SIZE],password[MAX_BUFFER_SIZE],host[MAX_BUFFER_SIZE],path[MAX_BUFFER_SIZE],filename[MAX_BUFFER_SIZE];

    if(parseInput(argv[1],user,password,host,path,filename)!=0){
        fprintf(stderr,"Error in file parsing\n");
        exit(-1);
    }

    fprintf(stdout,"%s\n%s\n%s\n%s\n%s\n",user,password,host,path,filename);

    
    fprintf(stdout,"Not implemented yet\n");
    return 0;
}


int parseInput(const char * input,char * user,char * password,char * host,char * path,char * filename){

    if(input==NULL||user==NULL||password==NULL||host==NULL||path==NULL||filename==NULL){
        fprintf(stderr,"Invalid References in parse input\n");
        return -1;
    }

    //Const to avoid erros
    const int size_of_input=strlen(input);

    char path_and_filename[2*MAX_BUFFER_SIZE];
    char *pointer_aux=NULL;
    
    STATE current_state=ST_READ_FTP;

    int iterator_insert=0;

    for(int i=0;i<size_of_input;i++){

        switch (current_state)
        {
        case ST_READ_FTP:

            if(i==0){
                
                if(input[i]!='f')
                    current_state=ST_FAIL;
            }
            else if(i==1){
                
                if(input[i]!='t')
                    current_state=ST_FAIL;

            }
            else if(i==2){
                
                if(input[i]!='p')
                    current_state=ST_FAIL;

            }
            else if(i==3){
                
                if(input[i]!=':')
                    current_state=ST_FAIL;

            }
            else if(i==4){
                
                if(input[i]!='/')
                    current_state=ST_FAIL;

            }      
            else if(i==5){
                
                if(input[i]!='/')
                    current_state=ST_FAIL;
                else
                    current_state=ST_READ_USER;

            }

            break;
  
        case ST_READ_USER:

            if(i==size_of_input-1)
                current_state=ST_FAIL;
            
            //Checks if the max buffer_size is targeted, prevent memory faults
            if(iterator_insert==MAX_BUFFER_SIZE)
                current_state=ST_FAIL;

            if(input[i]==':'){
                current_state=ST_READ_PASSWORD;
                //Reseta o iterador auxiliar
                iterator_insert=0;
            }else{
                //Insere caracter a caracter na string de output
                user[iterator_insert]=input[i];
                iterator_insert++;
            }


            break;
        
        case ST_READ_PASSWORD:

            if(i==size_of_input-1)
                current_state=ST_FAIL;
            
            //Checks if the max buffer_size is targeted, prevent memory faults
            if(iterator_insert==MAX_BUFFER_SIZE)
                current_state=ST_FAIL;

            if(input[i]=='@'){
                current_state=ST_READ_HOST;
                //Reseta o iterador auxiliar
                iterator_insert=0;
            }else{
                //Insere caracter a caracter na string de output
                password[iterator_insert]=input[i];
                iterator_insert++;
            }

            break;
        
        case ST_READ_HOST:

            if(i==size_of_input-1)
                current_state=ST_FAIL;
            
            //Checks if the max buffer_size is targeted, prevent memory faults
            if(iterator_insert==MAX_BUFFER_SIZE)
                current_state=ST_FAIL;

            if(input[i]=='/'){
                current_state=ST_READ_URL_PATH;
                //Reseta o iterador auxiliar
                iterator_insert=0;
            }else{
                //Insere caracter a caracter na string de output
                host[iterator_insert]=input[i];
                iterator_insert++;
            }



            break;

        case ST_READ_URL_PATH:
                
            //Checks if the max buffer_size is targeted, prevent memory faults
            if(iterator_insert==MAX_BUFFER_SIZE)
                current_state=ST_FAIL;

            //Insere caracter a caracter na string de output
            path_and_filename[iterator_insert]=input[i];
            iterator_insert++;


            break;
        
        case ST_FAIL:
            
            fprintf(stderr,"ST_FAIL WAS REACHED\n");
            return(-1);

            break;
        
        default:
            fprintf(stderr,"Default was reached\n");
            current_state=ST_FAIL;
            break;
        }

    }

    //Diferenciate path from password

    pointer_aux=strrchr(path_and_filename,'/');
    
    //There is no path specified
    if(pointer_aux==NULL){
        path=NULL;
        strcpy(password,pointer_aux);
    }
    else{
        //++ to skip the /
        pointer_aux++;
        strcpy(filename,pointer_aux);
        pointer_aux--;
        //Place a \0 in the 
        *pointer_aux='\0';
        strcpy(path,path_and_filename);
        fprintf(stdout,"Nao sei se leva o ultimo / ou nao. Pus que nao precisava\n");
        
    }






    return 0;
}