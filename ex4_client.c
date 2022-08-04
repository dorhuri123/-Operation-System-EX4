// Dor Huri 209409218
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/random.h>
#include <fcntl.h>
#include "string.h"
#include <signal.h>
#include <stdlib.h>
#define ERRORMESSAGE "ERROR_FROM_EX4\n"
#define SERVER "to_srv"
#define CLIENT "to_client_"
#define ERROR -1
#define CANCEL_ALARM 0
#define EXIT_NORMAL 0
#define MAXSIZE 150
/*****************************************************
 * this function starts when the client get a signal
 * of type SIGUSR1 that mean the server finish writing
 * the answer to file and in this function we print
 * the result and delete the file.
 *****************************************************/
void endProcess(int sig){
    //for canceling client alarm
    alarm(CANCEL_ALARM);
    int fd_server;
    char pid[MAXSIZE],answer[MAXSIZE] = {},temp[MAXSIZE] = CLIENT;
    char * server_file;
    //converting pid num to string
    sprintf(pid,"%d",getpid());
    //full file name
    server_file = strcat(temp, pid);
    //opening file
    if((fd_server = open(server_file,O_RDONLY,0777)) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //setting read to start of file
    if(lseek(fd_server,0,SEEK_SET) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //read answer from server
    if(read(fd_server,answer,MAXSIZE) == ERROR) {
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //close server file
    if(close(fd_server) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //delete server file
    if(remove(server_file) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //printing result to the screen
    printf("%s\n",answer);
    exit(EXIT_NORMAL);
}
/*****************************************************
 * this function starts when the client get a signal
 * of type SIGALRM that mean we get a timeout so we
 * print the appropriate massage and exit the program.
 *****************************************************/
void sig_alarm(int sig){
    printf("Client closed because no response was received from the server for 30 seconds\n");
    exit(EXIT_NORMAL);
}
int main(int argc,char** argv) {
    //checking if number of parameter isn't 5 if not exit
    if(argc != 5){
        printf("ERROR_FROM_EX4\n");
    }
    //setting the signal handler function
    signal(SIGUSR1,endProcess);
    signal(SIGALRM,sig_alarm);
    int i=0,j=2,fd_client,r;
    char pid[MAXSIZE];
    unsigned long int s;
    //try 10 time to create file to_srv if not already exist
    for (;i<10;i++) {
        //check if to_serv doesn't exist
        if (access(SERVER, F_OK) != 0){
            //open to_serv file
            if((fd_client= open("to_srv",O_RDWR | O_CREAT, 0777)) == ERROR){
                printf(ERRORMESSAGE);
                exit(EXIT_NORMAL);
            }
            //converting pid num to string
            sprintf(pid,"%d",getpid());
            //writing the process pid to file
            if (write(fd_client, pid,strlen(pid)) == ERROR) {
                printf(ERRORMESSAGE);
                exit(EXIT_NORMAL);
            }
            //for new line
            if (write(fd_client, "\n",1) == ERROR) {
                printf(ERRORMESSAGE);
                exit(EXIT_NORMAL);
            }
            //for rest of data the client write to file
            for(;j<5;j++) {
                //writing the data to server in file
                if (write(fd_client, argv[j],strlen(argv[j])) == ERROR) {
                    printf(ERRORMESSAGE);
                    exit(EXIT_NORMAL);
                }
                //for new line
                if (write(fd_client, "\n",1) == ERROR) {
                    printf(ERRORMESSAGE);
                    exit(EXIT_NORMAL);
                }
            }
            //closing to_serv file
            if(close(fd_client) == ERROR){
                printf(ERRORMESSAGE);
                exit(EXIT_NORMAL);
            }
            //sending signal to server indicating file is ready to read
            kill(atoi(argv[1]),SIGUSR2);
            //setting alarm for client timeout
            alarm(30);
            //waiting for signal
            pause();
        }
        //in case to_srv already exist in directory
        else
            //sleeping for random num of second 0-5
	    syscall(SYS_getrandom,&s,sizeof(unsigned long int),0);
            sleep((s%6));
    }
    if(i == 10){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    return 0;
}