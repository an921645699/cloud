#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define IPSTR "ipstr"
#define PORT "port"
#define LISMAX "lismax"


struct sock_info
{
    char ipstr[32];//"127.0.0.1"
    int port;
    int lismax;
};
 
int read_conf(struct sock_info* dt);

int socket_init();


