#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<openssl/md5.h>

#define  MD5_LEN  16 
#define  BUFF_SIZE 1024*16

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

void fun_md5(int fd,int slen,char buf[])
{
    MD5_CTX ctx;
    unsigned char md[MD5_LEN] = {0};
    MD5_Init(&ctx);

    long len = 0;
    char buff[ BUFF_SIZE ];
    while( (len = read(fd,buff,BUFF_SIZE )) > 0 )
    {
        MD5_Update(&ctx,buff,len);
    }
    MD5_Final(md,&ctx);
    char bf[3] = {0};
    sprintf(buf,"ok%d#",slen);
    for(int i = 0; i < MD5_LEN; i++ ){
        sprintf(bf,"%02x",md[i]);
        strncat(buf,bf,2);
    }
    strcat(buf,"#");
    return ;
}
//下载
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

    printf("确认下载 %s 请回复y/Y,否则按任意键取消下载\n",name);
    char yn[8];
    fgets(yn,8,stdin);
    if(yn[0] != 'y'&&yn[0]!='Y')
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
    
    int num = 0;
    int curr_size = 0;
    char rbuff[1024] = {0};
    
    int namesize = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    if( namesize > 0 )
    {

        char md[64] = {0};
        fun_md5(fd,namesize,md);
        send(c,md,strlen(md),0);
        if(recv(c,status,31,0) <= 0)
        {
            printf("ser close\n");
            return ;
        }
        if(strncmp(status,"ok",2) == 0)
        {
            send(c,"ok",2,0);
            curr_size = namesize;
            lseek(fd,namesize,SEEK_SET);
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
        }
        else
        {
            
            send(c,"ok",2,0);
            lseek(fd,0,SEEK_SET);
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
        }
    }
    else
    {
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
    }
    printf("\n");
    close(fd);
    return;
}
//上传
void send_file(int c,char* name,char* send_buff)
{
    if(name == NULL||send_buff == NULL)
    {
        return ;
    }
 
    int fd = open(name,O_RDONLY);
    if(fd == -1){
        return ;
    }
    int filesize = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    if(filesize==0){
        printf("没有该文件(空)\n");
        return ;
    }

    printf("file:%s filesie:%d \n",name,filesize);
    printf("确认上传 %s 请回复y/Y,否则按任意键取消上传\n",name);
    char yn[8];
    //scanf("%c",&yn);
    fgets(yn,8,stdin);
    if(yn[0] != 'y'&&yn[0]!='Y')
    {
        printf("已取消上传\n");
        send(c,"err",3,0);
        return ;
    }
    
    send(c,send_buff,strlen(send_buff),0);
    char status[32] = {0};
    if(recv(c,status,31,0)<=0)
    {
        printf("ser close\n");
        return ;
    }
    if(strncmp(status,"ok",2) != 0)
    {
        printf("ser err\n");
        return ;
    }
    memset(status,0,32);
    sprintf(status, "%d", filesize);
    send(c,status,strlen(status),0);
    if(recv(c,status,31,0)<=0)
    {
        printf("ser close\n");
        return ;
    }
    if(strncmp(status,"ok",2) != 0)
    {
        printf("ser err\n");
        return ;
    }

    int n =0;
    int curr_size = 0;
    char rbuff[1024] = {0};
    while((n = read(fd,rbuff,1024)) > 0){
        send(c,rbuff,n,0);
        curr_size += n;
        
        double f = curr_size*100/filesize;
        printf("上传%.2lf%%\r",f);
    }
    printf("\n");
    close(fd);
    return;
    
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
            send_file(sockfd,myargv[1],send_buff);
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
