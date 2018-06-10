#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

//update log file 
void updateLog(char *username,char *filename, char *destination){
  fflush(NULL);
  FILE *fileP;
  //create/append to updateLog file
  fileP = fopen("/var/www/html/updateLog.txt","a+");
  //get current time
  time_t now;
  time(&now);
  struct tm * timeinfo;
  timeinfo = localtime ( &now );
  //save date/time,user, filename transferred and its location to update file
  fprintf(fileP,"\n%s -->%s transferred file %s to %s\n",asctime (timeinfo),username,filename,destination);

  fflush(NULL);
  fclose(fileP);
  fflush(NULL);
}
