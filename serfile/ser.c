#include "socket.h"
#include "thread.h"


int main()
{
    int sockfd = socket_init();
    if(sockfd == -1)
    {
        printf("socket init err \n");
        exit(1);
    }
    
    while(1)
    {
        struct sockaddr_in caddr;
        int len = sizeof(caddr);
        int c = accept(sockfd,(struct sockaddr*)&caddr,&len);
        if(c < 0)
            continue;

        printf("accept: %d\n",c);
        thread_start(c);
    }
}
