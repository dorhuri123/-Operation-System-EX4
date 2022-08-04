// Dor Huri 209409218
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "string.h"
#include <signal.h>
#include <stdlib.h>
#define ERRORMESSAGE "ERROR_FROM_EX4\n"
#define ERRORMESSAGE_ZERO "CANNOT_DIVIDE_BY_ZERO"
#define SERVER "to_srv"
#define CLIENT "to_client_"
#define ERROR -1
#define CANCEL_ALARM 0
#define EXIT_NORMAL 0
#define MAXSIZE 150
#define TRUE 1
/*****************************************************
 * this function starts when the server get a signal
 * of type SIGALRM that mean we get a timeout so we
 * print the appropriate massage and exit the program.
 *****************************************************/
void sig_alarm(int sig){
    printf("The Server was closed because no service request was received for the last 60 seconds\n");
    exit(EXIT_NORMAL);
}
/*****************************************************
 * this function is for the child process to calculate
 * the result and write it to t_client_{client_pid} file
 * according to input we get from client file.
 *****************************************************/
void calculate(){
    FILE* client_file = fopen(SERVER,"r");
    char client[MAXSIZE],data[4][MAXSIZE],pid[MAXSIZE],out[MAXSIZE],temp[MAXSIZE] = CLIENT;
    char* client_output_file;
    int i = 0,fd_server;
    for (; i < 4; ++i) {
        //getting file info line by line
        fgets(client,MAXSIZE,client_file);
        //copying line to data
        strcpy(data[i],client);
    }
    //deleting file to_srv
    if(remove(SERVER) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //converting int to string
    sprintf(pid,"%d", atoi(data[0]));
    //full file name
    client_output_file = strcat(temp, pid);
    //open server file
    if((fd_server = open(client_output_file,O_RDWR | O_CREAT,0777)) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    int first_args = atoi(data[1]),second_args = atoi(data[3]),operator = atoi(data[2]),output;
    //in case dividing by zero
    if(operator == 4 &&  second_args == 0){
        //writing answer to file
        if(write(fd_server,ERRORMESSAGE_ZERO, strlen(ERRORMESSAGE_ZERO)) == ERROR){
            printf(ERRORMESSAGE);
            exit(EXIT_NORMAL);
        }
    }
    else {
        //calculating result according to the operator we get as input
        switch (operator) {
            case 1:
                output = first_args + second_args;
                break;
            case 2:
                output = first_args - second_args;
                break;
            case 3:
                output = first_args * second_args;
                break;
            case 4:
                output = first_args / second_args;
                break;
            default:
                break;
        }
        //converting int to string
        sprintf(out, "%d", output);
        //writing answer to file
        if (write(fd_server, strcat(out, "\n"), strlen(out)) == ERROR) {
            printf(ERRORMESSAGE);
            exit(EXIT_NORMAL);
        }
    }
    //sent signal to client indicating file is ready
    kill(atoi(data[0]),SIGUSR1);
    exit(EXIT_NORMAL);
}
/*****************************************************
 * this function starts when the client get a signal
 * of type SIGUSR2 that mean the client finish writing
 * the details to file and in this function we create
 * a child process to handle it and parent wait for
 * more requests.
 *****************************************************/
void sig_child(int sig) {
    //for canceling server alarm
    alarm(CANCEL_ALARM);
    //resting the signal handler for signal SIGUSR2
    signal(SIGUSR2,sig_child);
    int pid;
    //creating child process
    if((pid = fork()) == ERROR){
        printf(ERRORMESSAGE);
        exit(EXIT_NORMAL);
    }
    //child process
    if(pid == 0){
        //call the function that handle the current client
        calculate();
    }
    //parent process
    else
        //return to main to wait for more requests
        return;

}
int main() {
    if (access("to_srv", F_OK) == 0){
        if(remove("to_srv") == ERROR){
            printf(ERRORMESSAGE);
            exit(EXIT_NORMAL);
        }
    }
    //setting the signal handler function
    signal(SIGALRM,sig_alarm);
    signal(SIGUSR2,sig_child);
    //server waiting on requests
    while(TRUE){
        //setting alarm for server timeout
        alarm(60);
        //waiting for signal
        pause();
    }
}
