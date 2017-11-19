
#include <netinet/tcp.h>
#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<errno.h> 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
	 
	int optval = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
        printf("Cannot set SO_REUSEADDR option "
               "on listen socket (%s)\n", strerror(errno));
    // set TCP_NODELAY for sure
    optval = 1;
    if (setsockopt(socket_desc, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0)
        printf("Cannot set TCP_NODELAY option "
               "on listen socket (%s)\n", strerror(errno));
	 
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
	do {
		while( (read_size = recv(client_sock , client_message , 2000 , MSG_DONTWAIT)) > 0 )
		{
			//Send the message back to client
			write(client_sock , client_message , read_size);
		}
	} while (errno == EAGAIN);     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
     
    return 0;
}