#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <errno.h>


//locks specified location
void LockFolder(char * dir){

 //pervents read,write,execute
  char mode[] = "0700";
  int i;
  i = strtol(mode, 0, 8);

  //attempt lock
  if (chmod (dir,i) < 0)
  {
     //error locking
     char err[500];
     strcpy(err,"error locking: ");
     strcat(err, strerror(errno));
     printf("%s",err);
     exit(1);
  }
  else{
    //if lock is successful
    char msg[500];
    printf("Locking %s success\n",dir);
  }
}

//unlocks specified location
void UnlockFolder(char * dir){
  //Everybody can read, write to, or execute
  char mode1[] = "0777";
  int i = strtol(mode1, 0, 8);
  //attempt unlock
  if (chmod (dir,i) < 0)
  {
     //error unlocking
     char err[500];
     strcpy(err,"error unlocking: ");
     strcat(err, strerror(errno));
     exit(1);
  }
  else{
    //if unlock is successful
    char msg[500];
    strcpy(msg,"Unlocking success\n");
  }
}
