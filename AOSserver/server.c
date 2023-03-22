#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define SERVER_PORT 6666
#define QUEUELEN 20
#define BUF_SIZE 1024
#define FILE_NAME 512
#define ADDMAX 10

static char* students[] = {"ken" , "alice" , "barbie" , "andy" , "daniel" , "ethen"};
static char* AOS[] = {"ken" , "alice" , "barbie"};
static char* CSE[] = {"daniel" , "andy" , "ethen"};

typedef struct control{
    char *filename;
    char *owner;
    int AOSread;
    int AOSwrite;
    int CSEread;
    int CSEwrite;
    int state;
    struct control *next;
    }control;
    
    control *head = NULL;
 


void *create_thread(new_server_socket);
void *write_thread(new_server_socket);
void *read_thread(new_server_socket);
void *changemode_thread(new_server_socket);


int
main(int argc , char** argv)
{
    const char* cr = "create";
    const char* wr = "write";
    const char* rd = "read";
    const char* ch = "mode";
    

    
    struct sockaddr_in server_addr;
    bzero(&server_addr , sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    
    int server_socket = socket(AF_INET , SOCK_STREAM , 0);
    if(server_socket <0)
    {
        printf("create Failed\n");
        exit(1);
    }
    
    if(bind(server_socket , (struct sockaddr*)&server_addr , sizeof(server_addr)))
    {
        printf("bind Failed\n");
        exit(1);
    }
    
    if(listen(server_socket,QUEUELEN))
    {
        printf("server listen failed\n");
        exit(1);
    }
    
    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        
        int new_server_socket = accept(server_socket , (struct sockaddr*)&client_addr, &length);
        if(new_server_socket <0)
        {
            printf("Accept Failed\n");
            exit(1);
        }
        pthread_t t;
        
        char mode[BUF_SIZE];
        bzero(mode , sizeof(mode));

        
        length = recv(new_server_socket , mode , BUF_SIZE , 0);
        printf("actionmode : %s\n",mode);
        if(length < 0)
        {
            printf("Receive mode Failed\n");
            break;
        }
        
        int cret = strcmp(mode , cr );
        int wret = strcmp(mode , wr );
        int rret = strcmp(mode , rd );
        int mret = strcmp(mode , ch );
        
        int pthread_id;
        if(cret ==0 )
        {
             int thread = pthread_create(&t , NULL , create_thread , new_server_socket);
             if(thread == -1)
             {
                 printf("cthread create Failed");
                 break;
             }
        }

        if (wret ==0 )
        {
            int thread = pthread_create(&t , NULL , write_thread , new_server_socket);
            if(thread == -1)
            {
                printf("wthread create Failed");
                break;
            }
        }
        
        if(rret == 0 )
        { 
            int thread = pthread_create(&t , NULL , read_thread , new_server_socket);
            if(thread == -1)
            {
                printf("rthread create Failed");
                break;
            }
        }
        
        if(mret == 0 )
        {
            int thread = pthread_create(&t , NULL , changemode_thread , new_server_socket);
            if(thread == -1)
            {
                printf("cmthread create Failed");
                break;
            }
        }
    }
}
        
        
void *create_thread(new_server_socket)
{
    char name[6];
    char filename[FILE_NAME];
    char addition[ADDMAX];
    int fd , length , write_length , aret , cret , pret , fileret , exist;
    char bufdata[BUF_SIZE];
    char permission1[5] = "allow";
    char permission2[4] = "deny";
    bzero(bufdata , sizeof(bufdata));
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0);
    if(length < 0)
    {
        printf("Receive name Failed\n");
        pthread_exit(1);
    }
    strncpy(name , bufdata , 6);
    bzero(bufdata , sizeof(bufdata));
    printf("user : %s\n",name);
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0 ); //FILENAME
    if(length < 0)
    {
        printf("Receive filename Failed\n");
        pthread_exit(1);
    }
    strncpy(filename , bufdata , FILE_NAME);
    bzero(bufdata , sizeof(bufdata));
    printf("filename : %s\n",filename);
    
    length = recv(new_server_socket , bufdata , BUF_SIZE ,0 );  //ADDITION
    if(length < 0 )
    {
        printf("Receive addition Failed\n");
        pthread_exit(1);
    }
    strncpy(addition , bufdata , ADDMAX);
    bzero(bufdata , sizeof(bufdata));

    for(int a = 0 ; a < 3 ; a++){
        cret = strcmp(name , CSE[a]);
        if(cret == 0)
            break;
        aret = strcmp(name , AOS[a]);
        if(aret == 0)
            break;
    }
    
    control *current = head;
    while(current != NULL){
        if(fileret = strcmp(current -> filename , filename) == 0){
                printf("File is already exist\n");
                exist = 1;
                send(new_server_socket , permission2 , BUF_SIZE , 0);
                break;
        }
        current = current -> next;
    }
    
    if(exist != 1){
        fd = open(filename , O_CREAT | O_EXCL , 0777);
        if(fd == -1)
        {
            printf("file creating failed\n");
            send(new_server_socket , permission2 , BUF_SIZE , 0);
            close(new_server_socket);
            pthread_exit(1);
        }     
        send(new_server_socket , permission1 , BUF_SIZE , 0);
        control *new_control = (control*) malloc(sizeof(control));
    
        new_control -> filename = (char *)malloc(sizeof(char) * (strlen(filename) + 1));
        strcpy(new_control -> filename, filename);
    
        new_control -> owner = (char *)malloc(sizeof(char) * (strlen(name) + 1));
        strcpy(new_control -> owner, name);

        new_control -> next =NULL;    

        if(aret == 0){
            if(addition[2] == 'r')
                new_control -> AOSread = 1;
            else
                new_control -> AOSread = 0;
            if(addition[3] == 'w')
                new_control -> AOSwrite = 1;
            else
                new_control -> AOSwrite = 0;
            if(addition[4] == 'r')
                new_control -> CSEread = 1;
            else
                new_control -> CSEread = 0;
            if(addition[5] == 'w')
                new_control -> CSEwrite = 1;
            else
                new_control -> CSEwrite = 0;
        }
    
        if(cret == 0){
            if(addition[2] == 'r')
                new_control -> CSEread = 1;
            else
                new_control -> CSEread = 0;
            if(addition[3] == 'w')
                new_control -> CSEwrite = 1;
            else
                new_control -> CSEwrite = 0;
            if(addition[4] == 'r')
                new_control -> AOSread = 1;
            else
                new_control -> AOSread = 0;
            if(addition[5] == 'w')
                new_control -> AOSwrite = 1;
            else
                new_control -> AOSwrite = 0;
        }
    
        if(head == NULL){
            head = new_control;
        }         
        else{  
            control *current;
            current = head;
            while(1){
                if(current -> next == NULL){
                    current -> next = new_control;
                    break;
                }
                current = current -> next;
            }
        }
        printf("file sucessfully create\n");
        send(new_server_socket , permission1 , BUF_SIZE ,0);
        close(new_server_socket);
        pthread_exit(0);
    }
    
    if(exist == 1){
        close(new_server_socket);
        pthread_exit(1);
    }

}
void *write_thread(new_server_socket)
{
    char filename[FILE_NAME];
    char name[BUF_SIZE];
    char addition[ADDMAX];
    int fd , length , write_length , cret=1 , aret=1 , available , use ,clientret ;
    char bufdata[BUF_SIZE];
    char permission1[5] = "allow";
    char permission2[4] = "deny";
    char client[5];
    char* cover = "o";
    char* add = "a";
    bzero(bufdata , sizeof(bufdata));
    
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0);
    if(length < 0)
    {
        printf("Receive name Failed\n");
        pthread_exit(1);
    }
    strncpy(name , bufdata , 6);
    bzero(bufdata , sizeof(bufdata));
    printf("user : %s\n",name);
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0 ); //FILENAME
    if(length < 0)
    {
        printf("Receive filename Failed\n");
        pthread_exit(1);
    }

    strncpy(filename , bufdata , FILE_NAME);
    bzero(bufdata , sizeof(bufdata));
    printf("filename : %s\n",filename);
    
    length = recv(new_server_socket , bufdata , BUF_SIZE ,0 );  //ADDITION
    if(length < 0 )
    {
        printf("Receive addition Failed\n");
        pthread_exit(1);
    }
    strncpy(addition , bufdata , ADDMAX);
    printf("additionmode : %s\n",addition);
    bzero(bufdata , sizeof(bufdata));
    
    
    int coverret = strcmp(addition , cover );
    int addret = strcmp(addition , add );
    
    for(int a = 0 ; a < 3 ; a++){
        cret = strcmp(name , CSE[a]);
        if(cret == 0)
            break;
        aret = strcmp(name , AOS[a]);
        if(aret == 0)
            break;
    }
    control *current = head;

    int fileret;
    int nameret;
    
    if(current == NULL){
        printf("File is not exist\n");
        send(new_server_socket , permission2 , BUF_SIZE , 0);
        close(new_server_socket);
        pthread_exit(1);
    }
    
    while (current != NULL){
        if(fileret = strcmp(filename , current -> filename) == 0){
            if(nameret = strcmp(name , current -> owner) == 0){
                available = 1;
                break;
            }
            if(aret == 0){
                if(current -> AOSwrite == 1){
                    available =1;
                    break;
                }    
                if(current -> AOSwrite == 0){
                    printf("Not be admitted\n");
                    break;
                }
            }
            if(cret == 0){
                if(current -> CSEwrite == 1){
                    available =1;
                    break;
                }
                if(current -> CSEwrite == 0){
                    printf("Not be admitted\n");
                    break;
                }
            }
        }
        current = current -> next;
    }       
    
    if(current == NULL){
        printf("File is not exist\n");
        send(new_server_socket , permission2 , BUF_SIZE , 0);
        close(new_server_socket);
        pthread_exit(1);
    }
    else{
        send(new_server_socket , permission1 ,BUF_SIZE , 0);
    }
        
    sleep(2);
    if(available == 1){
        send(new_server_socket , permission1 , sizeof(permission1) , 0);
    }
    
    if(available != 1){
        send(new_server_socket , permission2 , sizeof(permission2) , 0);
        close(new_server_socket); 
        pthread_exit(1);
    }
    sleep(2);
    
    //current -> state = 1;
    if( current -> state != 0 && available == 1){
        use = 1;
        send(new_server_socket , permission2 , sizeof(permission2) , 0);
        printf("File is still in Used!\n");
        close(new_server_socket); 
        pthread_exit(1);
    }  
    if (use != 1){
        send(new_server_socket , permission1 , sizeof(permission1) , 0);
    }
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0);  
    strcpy(client , bufdata);
    bzero(bufdata , sizeof(bufdata));
    clientret = strcmp(client , "allow");
        
        
    if(clientret != 0){
        printf("File is not exist in client\n");
        close(new_server_socket);
        pthread_exit(1);
    }
    if(available == 1 && current -> state == 0){
        
        if(clientret == 0){
        current -> state = 2;
        if(addret == 0)
            fd = open(filename ,  O_APPEND | O_WRONLY ,0777);

        if(coverret == 0)
            fd = open(filename ,  O_TRUNC | O_WRONLY ,0777);
        
        if(fd==-1){
            printf("File open fail\n");
            current -> state = 0;
            close(new_server_socket);
            pthread_exit(1);
        }
    
        else{
            while (length = recv(new_server_socket , bufdata , BUF_SIZE , 0))
            {
                if(length < 0)
                {
                    printf("Receive Failed\n");
                    current -> state =0;
                    break;
                }

                write_length = write(fd , bufdata , length);
            
                if(write_length < length)
                {
                    printf("Write Failed\n");
                    current -> state =0;
                    break;
                }
            
                bzero(bufdata , BUF_SIZE);    
            }
            printf("State responsed / Process End\n");
            current -> state =0;
            close(fd);
            close(new_server_socket);
            pthread_exit(0);
        }
    }
    }
    close(new_server_socket);
    pthread_exit(1);
}

void *read_thread(new_server_socket)
{
    char filename[FILE_NAME];
    char name[BUF_SIZE];
    const char permission1[5] = "allow";
    const char permission2[4] = "deny";
    int fd , length , read_length , cret=1 , aret=1 , available , use;
    char bufdata[BUF_SIZE];
    bzero(bufdata , sizeof(bufdata));
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0);
    if(length < 0)
    {
        printf("Receive name Failed\n");
        pthread_exit(1);
    }
    strncpy(name , bufdata , 6);
    bzero(bufdata , sizeof(bufdata));
    printf("user : %s\n",name);
    
    length = recv(new_server_socket , bufdata , sizeof(bufdata) , 0 ); //FILENAME
    if(length < 0)
    {
        printf("Receive filename Failed\n");
        pthread_exit(1);
    }
    strncpy(filename , bufdata , FILE_NAME);
    bzero(bufdata , sizeof(bufdata));
    printf("filename : %s\n",filename);
    
    for(int a = 0 ; a < 3 ; a++){
        cret = strcmp(name , CSE[a]);
        if(cret == 0)
            break;
        aret = strcmp(name , AOS[a]);
        if(aret == 0)
            break;
    }
    control *current = head;
    int fileret;
    int nameret;
    
    if(current == NULL){
        printf("File is not exist\n");
        send(new_server_socket , permission2 , BUF_SIZE , 0);
        close(new_server_socket);
        pthread_exit(1);
    }
        
    
    while (current != NULL){
        if(fileret = strcmp(filename , current -> filename) == 0){
        send(new_server_socket , permission1 , BUF_SIZE , 0);
            if(nameret = strcmp(name , current -> owner) == 0){
                available = 1;
                break;
            }
            if(aret == 0){
                if(current -> AOSread == 1){
                    available = 1;
                    break;
                }    
                if(current -> AOSread == 0){
                    printf("Not be admitted\n");
                    available = 0;
                    break;
                }
            }
            if(cret == 0){
                if(current -> CSEread == 1){
                    available = 1;
                    break;
                }
                if(current -> CSEread == 0){
                    printf("NOT be admitted\n");
                    available = 0;
                    break;
                }
            }
        }
        current = current -> next;    
    }
    
    if(current == NULL){
        printf("File is not exist\n");
        send(new_server_socket , permission2 , BUF_SIZE , 0);
        close(new_server_socket);
        pthread_exit(1);
    }
    
    if(available == 1){
        send(new_server_socket , permission1 , sizeof(permission1) , 0);
        close(new_server_socket);
        pthread_exit(1);
    }
    
    if(available != 1){
        send(new_server_socket , permission2 , sizeof(permission2) , 0);
    }
    sleep(2);

    //current -> state = 2;
    if( current -> state == 2 && available == 1){
        printf("File is still in Used!\n");
        send(new_server_socket , permission2 , BUF_SIZE , 0);
        close(new_server_socket);
        pthread_exit(1);
    }

    else{
        send(new_server_socket , permission1 , sizeof(permission1) , 0);
    }
    sleep(2);
    if(available == 1 && current -> state != 2){
        current -> state = 1;
        fd = open(filename , O_RDONLY );
        if(fd == -1)
        {
            printf("File is not exist\n");
        }
        else
        {
            while((read_length = read(fd , bufdata , BUF_SIZE ))>0)
            {
                printf("%s, %d\n",bufdata,read_length);
                if(send(new_server_socket , bufdata , read_length , 0)<0)
                {
                    printf("Send Failed\n");
                    current -> state =0;
                    break;
                }
                bzero(bufdata , sizeof(bufdata));
            }
            printf("State responsed / Process End\n"); 
        }
        sleep(10);
        current -> state = 0;
        close(new_server_socket);
        close(fd);
        pthread_exit(0); 
    }
    close(new_server_socket);
    pthread_exit(1);
}

void *changemode_thread(int new_server_socket)
{
    char filename[BUF_SIZE];
    char name[BUF_SIZE];
    char addition[BUF_SIZE];
    char permission1[5] = "allow";
    char permission2[4] = "deny";
    int fd , length , write_length , available , cret , aret;
    char bufdata[BUF_SIZE];
    bzero(bufdata , sizeof(bufdata));
            
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0);
    if(length < 0)
    {
        printf("Receive name Failed\n");
        pthread_exit(1);
    }
    strncpy(name , bufdata , 6);
    bzero(bufdata , sizeof(bufdata));
    printf("user :%s\n",name);
    
    length = recv(new_server_socket , bufdata , BUF_SIZE , 0 ); //FILENAME
    if(length < 0)
    {
        printf("Receive filename Failed\n");
        pthread_exit(1);
    }
    strncpy(filename , bufdata , FILE_NAME);
    bzero(bufdata , sizeof(bufdata));
    printf("filename : %s\n",filename);
    
    length = recv(new_server_socket , bufdata , BUF_SIZE ,0 );  //ADDITION
    if(length < 0 )
    {
        printf("Receive addition Failed\n");
        pthread_exit(1);
    }
    strncpy(addition , bufdata , ADDMAX);
    bzero(bufdata , sizeof(bufdata));
    printf("New access right request : %s\n", addition);
    
    for(int a = 0 ; a < 3 ; a++){
        cret = strcmp(name , CSE[a]);
        if(cret == 0)
            break;
        aret = strcmp(name , AOS[a]);
        if(aret == 0)
            break;
    }
    
    control* current = head;
    int fileret;
    int nameret;
    if(current == NULL){
        printf("File is not exist\n");
        send(new_server_socket , permission2 , BUF_SIZE , 0);
        close(new_server_socket);
        pthread_exit(1);
    }
    while (current != NULL){
        if(fileret = strcmp(filename , current -> filename) == 0){
            send(new_server_socket , permission1 , BUF_SIZE , 0);
            if(nameret = strcmp(name , current -> owner) == 0){
                available = 1;
                send(new_server_socket , permission1 , 5 , 0);
                break;
            }
            else{
                printf("Not have the permission\n");
                send(new_server_socket , permission2 , BUF_SIZE , 0);
                break;
            }
        }
        current = current -> next;
        if(current == NULL){
            printf("File is not exist\n");
            send(new_server_socket , permission2 , BUF_SIZE , 0);
            break;
        }
    }
    
    if(available == 1){
    
        if(aret == 0){
            if(addition[2] == 'r')
                current -> AOSread = 1;
            else
                current -> AOSread = 0;
            if(addition[3] == 'w')
                current -> AOSwrite = 1;
            else
                current -> AOSwrite = 0;
            if(addition[4] == 'r')
                current -> CSEread = 1;
            else
                current -> CSEread = 0;
            if(addition[5] == 'w')
                current -> CSEwrite = 1;
            else
                current -> CSEwrite = 0;
        }
        if(cret == 0){
            if(addition[2] == 'r')
                current -> CSEread = 1;
            else
                current -> CSEread = 0;
            if(addition[3] == 'w')
                current -> CSEwrite = 1;
            else
                current -> CSEwrite = 0;
            if(addition[4] == 'r')
                current -> AOSread = 1;
            else
                current -> AOSread = 0;
            if(addition[5] == 'w')
                current -> AOSwrite = 1;
            else
                current -> AOSwrite = 0;
        }
        printf("State responsed / Process End\n");
        close(new_server_socket);
        pthread_exit(0);
        }
    close(new_server_socket);
    pthread_exit(1);
}
