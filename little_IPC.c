//static linking
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>

//global stuff
int received,expired_time;

//prototypes
void handler(int signal);

//text segment
int main(int args, char *argv[]) {

 //initialized/not initialized datas
 struct sigaction action;
 int descriptors[2];
 int number;
 char buffer[2];
 int pid,pid1;
 int n = atoi(argv[1]);
 received = 0;
 expired_time = 0;
 int file = open("prov_file",O_CREAT | O_RDWR,0644);

 //signal listener definition
 action.sa_handler = handler;
 sigemptyset(&action.sa_mask);
 action.sa_flags = SA_RESTART;
 if(sigaction(SIGUSR1, &action, NULL) == -1) {
  printf("An error occured with signal definition\n");
  exit(0);
 }
 if(sigaction(SIGALRM, &action, NULL) == -1) {
  printf("An error occured with signal definition\n");
  exit(0);
 }
 
 //pipe
 if(pipe(descriptors) != 0) {
  printf("An error occured creating pipe\n");
  exit(0);
 }  
 write(1,"Please wait for about ",22);
 sprintf(buffer,"%d",n);
 write(1,buffer,2);
 write(1," seconds, processes are communicating..\n",40); 

 //first son process
 pid = fork(); 
 if(pid == 0) {
  if(received == 0) pause();
  close(descriptors[0]);
  close(file);
  if(alarm(n) != 0) {
   printf("An error occured launching alarm\n");
   exit(0);
  }
  
  //IPC: writing through the pipe
  while(expired_time == 0) {
   number = rand()%50;
   sprintf(buffer,"%d",number);
   if(write(descriptors[1],buffer,2) < 0) {
    printf("An error occured in writing\n");
    exit(0);
   }
  }
  close(descriptors[1]);
  exit(0);
 }
 else {
   if(pid > 0) {
    if(kill(pid, SIGUSR1) != 0) {
      printf("An error occured during sending signal\n");
      exit(0);
    }
   }
   if(pid < 0) {
    printf("An error occured during forking process\n");
    exit(0);
   }
 }

 //second son process
 pid1 = fork();
 if(pid1 == 0) {   
   if(received == 0)  pause();
   close(descriptors[1]);

   //IPC: reading through the pipe
   while(read(descriptors[0],buffer,2) > 0) {
    if(write(file,buffer,2) < 0) {
     printf("An error occured in reading\n");
     exit(0);
    }
   }
   write(1,"Finished, they are writing the result of the dialogue from pipe to a file (redirection)\n",89);
   close(file);
   close(descriptors[0]);
   exit(0);
 }
 else {
  if(pid1 > 0) {
   //main process
   if(kill(pid1, SIGUSR1) != 0) {
    printf("An error occured during sending signal\n");
    exit(0);
   }
  }
  if(pid1 < 0) {
   printf("An error occured during forking process\n");
   exit(0);
  }
 }
 close(descriptors[0]);
 close(descriptors[1]);
 waitpid(pid,NULL,0);
 waitpid(pid1,NULL,0);
 
 //reading file
 write(1,"Reading redirected file:\n",26);
 lseek(file,0L,SEEK_SET);
 while(read(file,buffer,2) > 0) {
  if(write(1,buffer,2) < 0) {
   printf("An error occured in writing\n");
   exit(0);
  }
 }
 write(1,"\n",1); 
 unlink("prov_file");
 exit(0);
}

//handler
void handler(int signal) {
 
 if(signal == SIGUSR1) {
  received = 1;
 }
 if(signal == SIGALRM) {
  expired_time = 1;
 }
 return;
}
