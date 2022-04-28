#include "thread.h"
#include <openssl/md5.h>
#define MD5_LEN 16

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
        return ;
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

char* fun_md5(int fd,int slen)
{
    MD5_CTX ctx;
    unsigned char md[MD5_LEN] = {0};
    MD5_Init(&ctx);

    unsigned long len = 0;
    char buff[ slen+1 ];
    if(slen != read(fd,buff,slen )){
        return -1;
    }
    MD5_Update(&ctx,buff,slen);
    MD5_Final(md,&ctx);
    char tmp[3]={'\0'};
    char buf[64]={'\0'};
    for(int i = 0; i < MD5_LEN; i++ )
    {
        sprintf(tmp,"%02X",md[i]);
        strcat(buf,tmp);
    }
    return buf;
}
//发送
void send_file(int c,char* filename)
{
    if(filename == NULL){
        send(c,"get err",7,0);
        return ;
    }

    int fd = open(filename,O_RDONLY);
    if(fd == -1){
        send(c,"get err",7,0);
        return ;
    }

    int filesize = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    char status[64] = {0};
    sprintf(status,"ok#%d",filesize);
    send(c,status,strlen(status),0);
    memset(status,0,32);

    int num = recv(c,status,63,0);
    int n =0;
    char buff[1024] = {0};
    if(num <=0 || strncmp(status,"ok",2) != 0){     
        return ;
    }
    else if(num>2){
        char* s = strtok(status+2,"#");
        int slen = atoi(s);
        s = strtok(NULL,"#");
        char* md = fun_md5(fd,slen);
        md[strlen(md)-1] = 0;
        if(strcmp(md,s)==0){
            send(c,"ok",2,0);
            lseek(fd,slen,SEEK_SET);
            while((n = read(fd,buff,1024)) > 0){
                send(c,buff,n,0);
            }
        }
        else{
            send(c,"err",3,0);
            while((n = read(fd,buff,1024)) > 0){
                send(c,buff,n,0);
            }
        }
    }
    else{
        while((n = read(fd,buff,1024)) > 0){
            send(c,buff,n,0);
        }
    }
    
    close(fd);
}
//接收
void secv_file(int c,char* filename)
{
    if(filename == NULL)
    {
        send(c,"up err",7,0);
        return ;
    }

    int fd = open(filename,O_WRONLY|O_CREAT,0600);
    if(fd == -1)
    {
        send(c,"up err",7,0);
        return ;
    }
    send(c,"ok",2,0);
    char status[32] = {0};
    if(recv(c,status,31,0)<0)
    {
        return ;
    }
    int len = atoi(status);
    send(c,"ok",2,0);
    
    int num = 0,n = 0;
    char rbuff[1024] = {0};
    while(1)
    {
        n = recv(c,rbuff,1024,0);
        write(fd,rbuff,n);
        num += n;
        if(num>=len)
        {
            break;
        }
    }
    close(fd);  
}

void* thread_work(void* arg)
{
    int c = (int)arg;
     
    while(1)
    {
        char buff[128] = {0};
        int n = recv(c,buff,127,0);
        if(n <= 0){
            break;
        }

        char* myargv[ARG_MAX] = {0};
        char* cmd = get_cmd(buff,myargv);

        if( cmd == NULL)
        {
            send(c,"err",3,0);
            continue;
        }
        else if( strcmp(cmd,"get") == 0)
        {
            send_file(c,myargv[1]);
        }
        else if( strcmp(cmd,"up") == 0)
        {
            secv_file(c,myargv[1]);
        }
        else
        {
            do_run(c,cmd,myargv);            
        }
    }
    printf("cli(%d) close\n",c);
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
