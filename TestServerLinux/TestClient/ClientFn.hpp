#pragma once

extern "C" {  

    
int initsocket(const char* ip, int port);

int sendsocket(char* ptr, int length) ;

int recvsocket(char*ptr, int length) ;

int closesocket();
}