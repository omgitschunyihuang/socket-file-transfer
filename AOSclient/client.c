#include <netinet/in.h>  //sockaddr_in
#include <sys/types.h>   //socket
#include <sys/socket.h>  //socket
#include <stdio.h>       //printf
#include <stdlib.h>      //exit
#include <string.h>      //bzero
#include <fcntl.h>
#include <unistd.h>

#define SERVER_PORT 6666
#define BUF_SIZE 1024
#define FILE_NAME 512
#define IP_MAX 16
#define MODE_SIZE 15
#define ADD_SIZE 10

static char* students[] = {"ken" , "alice" , "barbie" , "andy" , "daniel" , "ethen"};

char mode[MODE_SIZE];
char filename[FILE_NAME];
char addition[ADD_SIZE];
char name[6];
int
main(){
    const char* cr = "create";
    const char* wr = "write";
    const char* rd = "read";
    const char* ch = "mode";
    
    int nameret ;
    
    const char* d = " ";
    bzero(name , sizeof(name));
    printf("please enter username:\n");
    scanf("%s",name);
    for(int i =0 ; i <6 ;i++){
        nameret = strcmp(name , students[i]);
        if(nameret == 0)
            break;
    }
    if(nameret != 0){
        printf("you are not be admitted\n");
        exit(1);
     }
        
    
    char buffer[BUF_SIZE];
    printf("enter mode filename addition(optional):\n");
    read(0,buffer,BUF_SIZE);
    
    int c = 0;
    char *p;
    char **string = NULL;
    p = strtok(buffer , d);
    
    while(p){ 
	c++;
	string = (char **)realloc(string , sizeof(char *) * c);
	string[c-1] = (char *)malloc(sizeof(char *) * (strlen(p)+1));

	string[c-1][0] = '\0';
	strcpy(string[c-1] , p );

	p = strtok((char *)NULL , d);
     }
     
     string = (char **)realloc(string , sizeof(char *) * (c + 1));
     string[c] = NULL;
         
     bzero(mode , sizeof(mode));
     strcpy(mode , string[0]);
     
     bzero(filename , sizeof(filename));
     strcpy(filename , string[1]);
     
     bzero(addition , sizeof(addition));
     if(string[2]!=NULL)
     strcpy(addition , string[2]);

        
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family=AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);
    
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if(client_socket<0)
    {
        printf("Create Socket Failed\n");
        exit(1);
    }
    
    if(bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Failed\n");
        exit(1);
    }
    
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    if(inet_aton("127.0.0.1",&server_addr.sin_addr)==0)
    {
        printf("Server IP error\n");
        exit(1);
    }
    
    server_addr.sin_port = htons(SERVER_PORT);
    socklen_t server_addr_lenth = sizeof(server_addr);
    
    if(connect(client_socket,(struct sockaddr*)&server_addr,server_addr_lenth)<0)
    {
        printf("Connect Failed %s\n",string[0]);
        exit(1);
    }
    
    send(client_socket , mode ,BUF_SIZE,0);
    int cret = strcmp(mode , cr );
    int wret = strcmp(mode , wr );
    int rret = strcmp(mode , rd );
    int mret = strcmp(mode , ch );

    if(cret ==0 )
        createfile(client_socket );
    
    if(wret == 0 )
        writefile( client_socket);
        
    if(rret == 0 )
        readfile(client_socket);
    
    if(mret == 0 )
        changemode( client_socket);
        
        
}
int
createfile( client_socket )
{   
    char bufdata[BUF_SIZE];
    int fileret1 , fileret2 ,length ; 
    char file1[5];
    char file2[5];
    send(client_socket , name , BUF_SIZE , 0);
    sleep(2);
    send(client_socket , filename ,BUF_SIZE , 0);
    sleep(2);
    send(client_socket , addition , BUF_SIZE , 0);
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0);
    if(length < 0){
        printf("Not have File exist announcement\n");
        exit(1);
    }
    strncpy(file1 , bufdata , 5);
    bzero(bufdata , sizeof(bufdata));
    fileret1 = strcmp(file1 , "allow");
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0);
    if(length < 0){
        printf("Not have File create announcement\n");
        exit(1);
    }
    strncpy(file2 , bufdata , 5);
    bzero(bufdata , sizeof(bufdata));
    fileret2 = strcmp(file2 , "allow");
    
    if(fileret1 != 0)
        printf("File is already exist\n");
    
    if(fileret2 != 0 && fileret1 == 0)
        printf("File creating is failed\n");

    close(client_socket);
    return 0;
}

int
writefile(client_socket)
{
    char bufdata[BUF_SIZE]; //DATA
    char permission1[5] = "allow";
    char permission2[4] = "deny";
    char permission[5];
    char use[6];
    char exist[5];
    int fd , read_length , length , allowret , useret ,existret;
    
    bzero(bufdata , BUF_SIZE);
    send(client_socket , name , BUF_SIZE , 0);
    sleep(2);
    send(client_socket , filename ,BUF_SIZE , 0);
    sleep(2);
    send(client_socket , addition , BUF_SIZE , 0);
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//exist
    if(length<0){
         printf("Not have file exist announcement\n");
         exit(1);
    }
    strncpy(exist , bufdata , 5);
    bzero(bufdata , sizeof(bufdata)); 
    existret = strcmp(exist , "allow");
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//permission
    if(length<0){
         printf("Not have permssion announcement\n");
         exit(1);
    }
    strcpy(permission , bufdata);
    bzero(bufdata , sizeof(bufdata)); 
    allowret = strcmp(permission , "allow");
    
    
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//use
    if(length<0){
        printf("Not have File state announcement\n");
        exit(1);
    }
    strncpy(use , bufdata , 5);
    bzero(bufdata , sizeof(bufdata));  
    useret = strcmp(use , "allow");

    
    if(allowret == 0 && useret == 0 && existret == 0){
    fd = open(filename , O_RDONLY);
    if(fd == -1)
    {
        printf("File is not exist in client\n");
        send(client_socket , permission2 , 5 , 0);
        exit(1);
    }
    else{
            send(client_socket , permission1 , 5 , 0);
            sleep(2);
            while((read_length = read(fd , bufdata , BUF_SIZE ))>0)
            {
                printf("%s, %d\n",bufdata,read_length);
                if(send(client_socket , bufdata , read_length , 0)<0)
                {
                    printf("Send Failed\n");
                    break;
                }
                bzero(bufdata , sizeof(bufdata));
            }
            bzero(bufdata , sizeof(bufdata));
            printf("Send Complete\n"); 
            close(fd);
            return 0;
        } 
    }
    if(allowret != 0 && existret == 0)
       printf("Not be admitted\n");
  
    if(useret != 0 && existret == 0 && allowret == 0)
       printf("File is still in Used!\n");
       
    if(existret != 0)
       printf("File is not exist in server\n");
       
    exit(1);       
}

int
readfile(client_socket)
{
    char bufdata[BUF_SIZE];
    char permission[5];
    char use[6];
    char exist[5];
    int fd , length , write_length , allowret , useret , existret;
    bzero(bufdata , sizeof(bufdata));
    send(client_socket , name , BUF_SIZE , 0);
    sleep(2);
    send(client_socket , filename ,BUF_SIZE , 0);
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//exist
    if(length<0){
         printf("Not have file exist announcement\n");
         exit(1);
    }
    strncpy(exist , bufdata , 5);
    bzero(bufdata , sizeof(bufdata));
    existret = strcmp(exist , "allow");
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//permission
    if(length<0){
         printf("Not have permssion announcement\n");
         exit(1);
    }
    strcpy(permission , bufdata);
    bzero(bufdata , sizeof(bufdata)); 
    allowret = strcmp(permission , "allow");
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//use
    if(length<0){
        printf("Not have File state announcement\n");
        exit(1);
    }
    strncpy(use , bufdata , 5);
    bzero(bufdata , sizeof(bufdata));  
    useret = strcmp(use , "allow");
    
    if(allowret == 0 && useret == 0 && existret == 0){
        fd = open(filename , O_CREAT | O_TRUNC | O_WRONLY ,0777);
        if(fd == -1)
        {
            printf("Can't open the file to write\n");
            exit(1);
        }
        else{
                while (length = recv(client_socket , bufdata , BUF_SIZE , 0))
                {
                    if(length < 0)
                    {
                        printf("Receive Failed\n");
                        break;
                    }
   
                    write_length = write(fd , bufdata , length);
               
                    if(write_length < length)
                    {
                        printf("Read Failed\n");
                        break;
                    }
                    bzero(bufdata , BUF_SIZE);    
                }
            printf("Sucessfully receive\n");
            close(fd);
            close(client_socket);
            return 0;
        } 
      }
      if(allowret != 0 && existret == 0)
          printf("Not be admitted\n");
  
      if(useret != 0 && existret == 0 && allowret == 0)
          printf("File is still in Used!\n");

      if(existret != 0)
          printf("File is not exist\n");
          
      exit(1);
}      


int
changemode(client_socket)
{
    char permission[5];
    char exist[5];
    char bufdata[BUF_SIZE];
    int allowret , length , existret;
    send(client_socket , name , BUF_SIZE , 0);
    sleep(2);
    send(client_socket , filename , BUF_SIZE , 0);
    sleep(2);
    send(client_socket , addition , BUF_SIZE , 0);
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//exist
    if(length<0){
         printf("File is not exist\n");
         exit(1);
    }
    strncpy(exist , bufdata , 5);
    bzero(bufdata , sizeof(bufdata));
    
    existret = strcmp(exist , "allow");
    
    length = recv(client_socket , bufdata , BUF_SIZE , 0 );//permission
    if(length<0){
         printf("NO permssion announcement\n");
         exit(1);
    }

    strncpy(permission , bufdata , 5);
    
    allowret = strcmp(bufdata , "allow");
    bzero(bufdata , sizeof(bufdata));
    if(allowret == 0 && existret == 0)
        printf("Sucessfully change\n");

    if(allowret != 0 && existret == 0)
        printf("Not be admitted\n");
        
    if(existret != 0)
        printf("File is not exist\n");
           
    close(client_socket);
    return(0);
}
