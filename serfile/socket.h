#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define IPSTR "ipstr"
#define PORT "port"
#define LISMAX "lismax"

struct sock_info
{
    char ipstr[32];
    int port;
    int lismax;
};

int read_conf();

int socket_init();


