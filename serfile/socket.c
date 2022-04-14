#include "socket.h"


int read_conf(struct sock_info * dt)
{
    FILE *fp = fopen("my.conf","r");
    if(fp == NULL)
    {
        return -1;
    }
    int index = 0;
    char buff[256] = {0};
    while( fgets(buff,256,fp) != NULL)
    {
        index++;
        if( strncmp(buff,"#",1) == 0 || strncmp(buff,"\n",1) == 0)
        {
            continue;
        }
        
    
        buff[strlen(buff)-1] = 0;

        if( strncmp(buff,IPSTR,strlen(IPSTR)) == 0)
        {
            strcpy(dt->ipstr,buff+strlen(IPSTR)+1);
        }
        else if( strncmp(buff,PORT,strlen(PORT)) == 0)
        {
            dt->port = atoi(buff+strlen(PORT)+1);
        }
        else if( strncmp(buff,LISMAX,strlen(LISMAX)) == 0)
        {
            dt->lismax = atoi(buff+strlen(LISMAX)+1);
        }
        else
        {
            printf("配置文件第%d行错误:%s\n",index,buff);
        }
    }    
    close(fp);
    return 0;    
}


int socket_init()
{
    struct sock_info sock_conf;
    if( read_conf(&sock_conf) == -1)
    {
        return -1;
    }
    printf("ip:%s\n",sock_conf.ipstr);
    printf("port:%d\n",sock_conf.port);
    printf("lismax:%d\n",sock_conf.lismax);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
        return -1;
    
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(sock_conf.port);
    saddr.sin_addr.s_addr = inet_addr(sock_conf.ipstr);

    int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(res == -1)
        return -1;

    res = listen(sockfd,sock_conf.lismax);
    if(res == -1)
        return -1;
    
    return sockfd;
}   
