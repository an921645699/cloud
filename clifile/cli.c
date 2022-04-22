#include<stdio.h>
#include<fcntl.h>
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

void recv_file(int c,char* name,char* send_buff)
{
    if(name == NULL||send_buff == NULL)
    {
        return ;
    }
    send(c,send_buff,strlen(send_buff),0);

    char status[32] = {0};
    if( recv(c,status,31,0) <= 0)
    {
        printf("ser close\n");
        return ;
    }
    
    if(strncmp(status,"ok#",3) != 0)
    {
        printf("文件不存在\n");
        return ;
    }
    
    int filesize = atoi(status+3);
    printf("file:%s filesie:%d \n",name,filesize);

    printf("确认下载 %s 请回复y/Y，否则按任意键取消下载\n",name);
    char yn;
    scanf("%c",&yn);
    if(yn != 'y'&&yn!='Y')
    {
        printf("已取消下载\n");
        send(c,"err",3,0);
        return ;
    }
    
    int fd = open(name,O_WRONLY|O_CREAT,0600);
    if(fd == -1)
    {
        send(c,"err",3,0);
        return ;
    }
    int namesize = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    int num = 0;
    int curr_size = 0;
    char rbuff[1024] = {0};
    if(namesize != 0)
    {
        char sbuff[10] = {0};
        strcpy(sbuff,"ok"); 
        read(fd,sbuff+2,8);
        send(c,sbuff,strlen(sbuff),0);
        memset(sbuff,0,10);
        if(recv(c,sbuff,9,0)<=0)
        {
            send(c,"err",3,0);
            return ;
        }
        if(strncmp(sbuff,"yes",3) == 0)
        {
            memset(sbuff,0,10);
            sprintf(sbuff,"ok%d",namesize);
            send(c,sbuff,strlen(sbuff),0);
            memset(rbuff,0,1024);
            curr_size = namesize;
            while(1)
            {
                num = recv(c,rbuff,1024,0);
                write(fd,rbuff,num);
                curr_size += num;

                double f = curr_size*100/filesize;
                printf("下载%.2lf%%\r",f);
                if(curr_size>=filesize)
                {
                    break;
                }
            }
            printf("\n");
            close(fd);
            return;
        }
    }
    send(c,"ok",2,0);
    while(1)
    {
        num=recv(c,rbuff,1024,0);
        write(fd,rbuff,num);
        curr_size += num;
        
        double f = curr_size*100/filesize;
        printf("下载%.2lf%%\r",f);
        if(curr_size>=filesize)
        {
            break;
        }
    }

    printf("\n");
    close(fd);
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
        char* cmd = get_cmd(buff,myargv);
        
        if( cmd == NULL)
            continue;

        else if(strcmp(cmd,"exit") == 0)
        {   
            printf("close\n");
            break;
        }
        else if( strcmp(cmd,"get") == 0)
        {

            recv_file(sockfd,myargv[1],send_buff);
        }
        else if( strcmp(cmd,"up") == 0)
        {
        }
        else if( strcmp(cmd,"ls") == 0 || strcmp(cmd,"rm") == 0||
                strcmp(cmd,"touch") == 0||strcmp(cmd,"mkdir") == 0||
                strcmp(cmd,"mv") == 0||strcmp(cmd,"cp") == 0)
        {     
            send(sockfd,send_buff,strlen(send_buff),0);
            char rbuff[128] = {0};
            if( recv(sockfd,rbuff,127,0) <= 0)
            {
                printf("ser close\n");
                break;
            }
            if( strncmp(rbuff,"ok#",3) == 0)
            {
                printf("%s\n",rbuff+3);
            }
            else
            {
                printf("服务器错误:%s\n",rbuff);
            }
        }
        else
        {
            printf("(我不懂你什么意思，请重新输入)\n");
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
