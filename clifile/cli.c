#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

int socket_create();
char* get_cmd(char buff[],char* myargv[])
{
    if(buff == NULL || myargv == NULL)
        return NULL;

    int index = 0;
    char *s = strtok(buff," ");
    while( s != NULL)
    {
        myargv[index++] = s;
        s = strtok(NULL," ");
    }

    return myargv[0];
}

int main()
{
    int sockfd = socket_create();
    if(sockfd == -1)
        exit(0);
    printf("connect >>\n");
    fflush(stdout);
    while(1)
    {

        printf("input\n");
        char buff[128]={0};
        fgets(buff,127,stdin);
        buff[strlen(buff)-1] = 0;
        
        char send_buff[128] = {0};
        strcpy(send_buff,buff);
        
        char* myargv[10] = {0};
        char cmd = get_cmd(buff,myargv);

        if( cmd == NULL)
            continue;

        else if(strcmp(cmd,"exit") == 0)
        {   
            printf("ser close");
            break;
        }
        else if( strcmp(cmd,"get") == 0)
        {
        }
        else if( strcmp(cmd,"up") == 0)
        {
        }
        else
        {        
            send(sockfd,send_buff,strlen(buff),0);
            memset(send_buff,0,128);
            if( recv(sockfd,send_buff,127,0)<=0)
            {
                printf("ser close\n");
                break;
            }
            if( strncmp(send_buff,"ok#",3) == 0)
            {
                printf("%s\n",send_buff+3);
            }
            else
            {
                printf("服务器错误:%s\n",send_buff);
            }
        }
    }
    close(sockfd);
    return 0;
}
int socket_create()
{
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
        return -1;

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(6000);
    saddr.sin_addr.s_addr=inet_addr("127.0.0.1");

    int res=connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(res == -1)
    {
        printf("connect err\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}
