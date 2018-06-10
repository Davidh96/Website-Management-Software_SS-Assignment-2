#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

#include "networking.h"
#include "log.h"
#include "AccessControl.h"

pthread_mutex_t mutex;
char message[500];

//holds connection information
struct connDetails{
  int s;
  int connSize;
};

//function deals with sending messages
void sendMessage(int cs,char * message){
	write(cs,message,strlen(message));
}

//function reads a message and returns message
char * readMessage(int cs){

  int READSIZE;
  //char * test;

  //clear read buffer
  memset(message,0,500);
  READSIZE = recv(cs,message,2000,0);
  //display client message
  printf("Client %d said: %s\n",cs,message);
  //if user cancels connection
  if(READSIZE==0){
    puts("Client disconnected");
    fflush(stdout);
    //close client file descriptor
    close(cs);
    //exit thread
    pthread_exit(0);
  }else if(READSIZE==-1){
    perror("read error");
  }

  //return pointer to message
  return &(message[0]);

}

//verify user credentials
char * verifyUser(int cs,char * userName){
  int verifyCheck=0;
  char * message;

  //while user is not verified
  while(verifyCheck==0){

    sendMessage(cs,"Enter Username");

    //place username into new var for user details
    message = readMessage(cs);
    char *user1;
    user1 = malloc(sizeof(char) * (strlen(message)+1));

    //place username into new var
    userName = malloc(sizeof(char) * (strlen(message)+1));
    strcpy(user1,message);
    strcpy(userName,message);

    sendMessage(cs,"Enter Password");

    message = readMessage(cs);

    //add password to user details string
    strcat(user1,", ");
    strcat(user1,message);
    strcat(user1,"\n");

    //check if user is in the users file
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    fp = fopen("./users.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    //check line by line
    while ((read = getline(&line, &len, fp)) != -1) {
        //if user found then break from loop
        if(strcmp(user1,line)==0){
          printf("Client %d Verified\n",cs);
          fflush(stdout);
          strcpy(message,"User Verified\nEnter file location: ");
          sendMessage(cs,message);
          verifyCheck++;
          break;
        }
    }

    fclose(fp);
    if (line){
        free(line);
    }
  }

  //return verified username
  return userName;
}

//deals with a client
void *serverLoop(void * clientSock){
  int READSIZE;
  char * message1;
  char *token;
  char *tempFile;
  char s[2]="/";
  char *userName;
  char fileName[500];
  char websiteLoc[500];
  char file_buffer[512]; // Receiver buffer
  int cs;
  pthread_t tid;
  cs = *(int *)clientSock;

  //get thread id
  tid=pthread_self();
  printf("Client %d on Thread ID: %d\n",cs,tid);
  fflush(stdout);
  fflush(stdin);

  //verify user and get username
  userName = verifyUser(cs,userName);

  //get file location
  message1 = readMessage(cs);

  //get file name, break up by '/'
  token = strtok(message1,s);

  //get last entry
  while(token!=NULL){
    tempFile = malloc(sizeof(char) * (strlen(token)+1));
    strcpy(tempFile,token);
  	token = strtok(NULL,s);
  }


  //strcat(fileName,"/");
  strcat(fileName,tempFile);

  //get destination for file
  //while a valid destination has not been entered
  while(atoi(message1)<1 || atoi(message1)>5){
    sendMessage(cs,"Enter Destination:\n1 for Root(/)\n2 for sales\n3 for promotions\n4 for offers\n5 for marketing");

    message1 = readMessage(cs);
  }


  strcpy(websiteLoc,"/var/www/html/intranet");
  //switch statment to deal with user choosing file destination
  switch(atoi(message1)){
    case 1:
      printf("Client %d: Root selected\n",cs);
      strcat(websiteLoc,"/");
      break;
    case 2:
      printf("Client %d: Sales selected\n",cs);
      strcat(websiteLoc,"/sales/");
      break;
    case 3:
      printf("Client %d: Promotions selected\n",cs);
      strcat(websiteLoc,"/promotions/");
      break;
    case 4:
      printf("Client %d: Offers selected\n",cs);
      strcat(websiteLoc,"/offers/");
      break;
    case 5:
      printf("Client %d: Marketing selected\n",cs);
      strcat(websiteLoc,"/marketing/");
      break;

  }

  sendMessage(cs,"Transfer File");

  //lock resource during transfer
  pthread_mutex_lock(&mutex);

  //recieve file from client
  char* file_name = strcat(websiteLoc,fileName);

  FILE *file_open = fopen(file_name, "w");
  if(file_open == NULL){
    fflush(stdout);
    //return to client that transfer failed
    sendMessage(cs,"Unable to transfer file");
    fflush(stdout);
    printf("File %s Cannot be opened file on server.\n", file_name);
    fflush(stdout);
    //end connection
    close(cs);
    pthread_exit(0);
  }else {
    bzero(file_buffer, 512);
    int block_size = 0;
    int i=0;
    while((block_size = recv(cs, file_buffer, 512, 0)) > 0) {
       printf("Client %d: Data Received, %d = %d\n",cs,i,block_size);
       int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
       //program gets stuck if block less than 512 is recieved
       if(block_size<512){
         break;
       }

       bzero(file_buffer, block_size);
       i++;
    }

    fflush(stdout);
    //return to client that transfer was successful
    sendMessage(cs,"Transfer was successful");
    fflush(stdout);
    printf("Client %d: Transfer Complete\n",cs);
    fflush(stdout);
  }

  fclose(file_open);

  updateLog(userName,fileName,websiteLoc);

  //to test mutex locking
  printf("Hit enter to continue");
  getchar();

  //unlock mutex
  pthread_mutex_unlock(&mutex);

  //end connection
  close(cs);
  pthread_exit(0);
}

int main(int argc, char *argv[]){

  int cs;
  struct sockaddr_in client;
  int rc;

  //lock folders
  LockFolder("/var/www/html/intranet");
  LockFolder("/var/www/html/live");

  //initialize mutex
  pthread_mutex_init(&mutex,NULL);

  //server setup
  struct connDetails details = setupServer();

  //run infinite loop, wait for connections
  while(1){
  	  //accept connection
      cs = accept(details.s,(struct sockaddr *)&client,(socklen_t*)&details.connSize);

      if(cs<0){
        perror("Can't establish connecton\n");
        return 1;
      }else{
        printf("Connection accepted\n");
      }

      //create new thread for new connection
      pthread_t newThread;
      //create new socket to handle connection
      int *newCS;
      newCS = malloc(sizeof *newCS);
      *newCS = cs;

      //create thread and start thread function
      if(rc = pthread_create(&newThread,NULL,serverLoop,(void *)newCS)<0){
        printf("Thread Error");
        return EXIT_FAILURE;
      }

    }

}
