#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<fcntl.h>


#define READ_BUFF 1024
#define ARG_MAX 10

void do_run(int c,char* cmd,char* myargv[]);

void* thread_work(void* arg);

char* get_cmd(char buff[],char* myargv[]);

void thread_start();
