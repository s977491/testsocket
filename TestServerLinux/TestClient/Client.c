#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<time.h>
#include<errno.h> 
#include <netinet/tcp.h>

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8888 );
 
 int optval = 1;
    // set TCP_NODELAY for sure
    optval = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0)
        printf("Cannot set TCP_NODELAY option "
               "on listen socket (%s)\n", strerror(errno));
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    //keep communicating with server
    while(1)
    {
        printf("Enter message : ");
        scanf("%s" , message);
         
        //Send some data
		struct timespec tstart={0,0}, tend={0,0};
		clock_gettime(CLOCK_MONOTONIC, &tstart);
   
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        while( recv(sock , server_reply , 2000 , MSG_DONTWAIT) < 0)
        {
			if (errno == EAGAIN) continue;
            puts("recv failed");
            break;
        }
		clock_gettime(CLOCK_MONOTONIC, &tend);
		long long ret = (tend.tv_sec * 1e9 +tend.tv_nsec) - (tstart.tv_sec * 1e9+tstart.tv_nsec);
		
		printf("some_long_computation took about %lld seconds\n",ret);
         
        puts("Server reply :");
        puts(server_reply);
    }
     
    close(sock);
    return 0;
}