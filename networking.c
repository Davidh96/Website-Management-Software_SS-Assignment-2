#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>

//holds connection information
struct connDetails{
  int s;
  int connSize;
};

//sets up server
struct connDetails setupServer(){
  int s;
  int connSize;
  struct sockaddr_in server;
  struct connDetails details;

  //create socket
  s = socket(AF_INET,SOCK_STREAM,0);

  if(s==-1){
    printf("Could not create socket\n");
  }else{
    printf("Socket created\n");
  }

  //set server details
  server.sin_port = htons(8081);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;

  //bind socket
  if(bind(s,(struct sockaddr *)&server,sizeof(server))<0){
    perror("Bind Issue\n");
    exit(0);
  }else{
    printf("Bind OK\n");
  }

  //listen for a connection
  listen(s,3);

  connSize=sizeof(struct sockaddr_in);

  //save connection details in the strcut
  details.s=s;
  details.connSize=connSize;

  printf("_______________________\n");
  fflush(NULL);

  //return connection details
  return details;
}

//setup connection to server
int setupClient(){
	int SID;
	struct sockaddr_in server;

  //create socket
	SID = socket(AF_INET, SOCK_STREAM,0);

	if(SID==-1){
		printf("Error creating socket");
	}else{
		printf("Socket Created\n");
	}

	//server details
	server.sin_port = htons(8081);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;

  //connect to server
	if(connect(SID,(struct sockaddr *)&server,sizeof(server))<0){
		printf("Connection failed\n");
		return 1;
	}
	else{
		printf("Connected successfully\n");
	}
  printf("_______________________\n");
  fflush(NULL);

	return SID;
}
