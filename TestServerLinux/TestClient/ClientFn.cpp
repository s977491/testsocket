
extern "C" {  

#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <netinet/tcp.h>
#include<time.h>
#include<errno.h> 
#include <unistd.h>
int sock;
    
int initsocket(const char* ip, int port)
{
    struct sockaddr_in server;
   
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
		return -1;
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return -2;
    }
     
    puts("Connected\n");
	
	return 0;     
}

int sendsocket(char* message, int length) {
	
	int sent = send(sock , message , length , MSG_DONTWAIT);
	if (sent < 0)		
	{
		if ( errno == EAGAIN) {
			puts("Send blocked ");
			return -1;
		}
		puts("Send failed ");
			
		return -2;
	}	
	return sent;
}

int recvsocket(char*server_reply, int length) {
	
	int recved = recv(sock , server_reply , length , MSG_DONTWAIT);
	if (recved <0) {
		if (errno == EAGAIN) {
			return -1;
		}
		puts("recv failed ");		
		return -2;
	}
	return recved;
}

int closesocket() {
    close(sock);
    return 0;	
		
}
}