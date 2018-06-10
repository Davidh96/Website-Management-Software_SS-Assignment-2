#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "networking.h"

//setup connection to server
int setupConnection(){
	int SID;
	struct sockaddr_in server;

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

	if(connect(SID,(struct sockaddr *)&server,sizeof(server))<0){
		printf("Connection failed\n");
		return 1;
	}
	else{
		printf("Connected successfully\n");
	}

	return SID;
}

//send a selected file to the selected server
void sendFile(int SID,char * fileLocation){
	char* file_name = fileLocation;
	char option[2], response[10];
	fflush(NULL);

	char file_buffer[512];

	memset(file_buffer, 0, sizeof(file_buffer));
	printf("Sending %s to the Server... ", file_name);
	FILE *file_open = fopen(file_name, "r");
	bzero(file_buffer, 512);
	int block_size,i=0;
	while((block_size = fread(file_buffer, sizeof(char), 512, file_open)) > 0) {
		printf("Data Sent %d = %d\n",i,block_size);
		if(send(SID, file_buffer, block_size, 0) < 0) {
		 exit(1);
		}

		bzero(file_buffer, 512);
		i++;
	}

fclose(file_open);

}

int main(int argc, char *argv[])
{

	int SID;
	char clientMessage[500];
	char serverMessage[500];

	//setup connection
	//SID = setupConnection();
	SID=setupClient();

	char fileLocation[500];

	do{
		memset(serverMessage, 0, sizeof(serverMessage));
		if(recv(SID,serverMessage,500,0)<0){
			printf("IO error");
			exit(1);
		}

		printf("Server said: %s: ",serverMessage);

		if(strcmp(serverMessage,"close")==0){
			break;
		}

		if(strcmp(serverMessage,"Transfer File")==0){
			break;
		}

		scanf("%s",clientMessage);

		if(strcmp("User Verified\nEnter file location: ",serverMessage)==0){
			int exists=0;

			//while the file entered does not exist
			while(exists==0){
				//check if file entered exists
				FILE *file;
				if ((file = fopen(clientMessage, "r")))
				{
					fclose(file);
					exists++;
				}else{
					//if file does not exist, get user to enter new file
					printf("File does not exist, please try again.\nEnter file location: ");
					scanf("%s",clientMessage);
				}
				//place location into new variable
				strcpy(fileLocation,clientMessage);
			}
		}

		//send client message to server
		if(send(SID,clientMessage,strlen(clientMessage),0)<0){
			printf("Send failed");
			return 1;
		}
	}while(strcmp(serverMessage,"close")!=0);

	sendFile(SID,fileLocation);

	fflush(stdin);
	fflush(stdout);
	fflush(NULL);

	//recieve last message from server
	memset(serverMessage, 0, sizeof(serverMessage));
	if(recv(SID,serverMessage,500,0)<0){
		printf("IO error");
		exit(1);
	}
	printf("%s",serverMessage);

	close(SID);
	return 0;
}
