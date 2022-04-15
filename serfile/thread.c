#include "thread.h"

void do_run(int c,char* cmd,char* myargv[])
{
    int fd[2];
    if( pipe(fd) == -1)
    {
        send(c,"pipe err",8,0);
        return ;
    }

    pid_t pid = fork();
    if(pid == -1)
    {
        send(c,"fork err",8,0);
    }
    if(pid == 0)
    {
        close(fd[0]);
        dup2(fd[1],1);
        dup2(fd[1],2);
        
        execvp(cmd,myargv);
        printf("exec err\n");
        exit(0);
    }
    close(fd[1]);
    wait(NULL);
    char read_buff[READ_BUFF] = "ok#";
    read(fd[0],read_buff+3,READ_BUFF-4);
    close(fd[0]);

    send(c,read_buff,strlen(read_buff),0);
}


void* thread_work(void* arg)
{
    int c = (int)arg;
     
    while(1)
    {
        char buff[128] = {0};
        int n = recv(c,buff,127,0);
        if(n < 0)
            break;
            
        char myargv[ARG_MAX] = {0};
        char cmd = get_cmd(buff,myargv);

        if( cmd == NULL)
        {
            send(c,"err",3,0);
            continue;
        }
        else if( strcmp(cmd,"get") == 0)
        {
            //下载
        }
        else if( strcmp(cmd,"up") == 0)
        {
            //上传
        }
        else
        {
            do_run(c,cmd,myargv);            
        }
    }
    close(c);
}

char* get_cmd(char buff[],char* myargv[])
{
    if(buff == NULL || myargv == NULL)
        return NULL;

    int index = 0;
    char * str = NULL;
    char* s=strtok_r(buff," ",&str);
    while( s != NULL)
    {
        myargv[index++] = s;
        s = strtok_r(NULL, " ",&str);
    } 
    return myargv[0];
} 

void thread_start(int c)
{
    pthread_t id;
    pthread_create(&id,NULL,thread_work,(void*)c);
}
