#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstring>
int main (){
    constexpr const char* port="80";
    std::string host="v2.jokeapi.dev";
    std::string path="/joke/Any";
    std::string host2="localhost";
    constexpr const char*port2="28333";
    struct addrinfo hints,*res;
    std::memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    int rv=getaddrinfo(host.c_str(),port,&hints,&res);
    if (rv!=0){
        std::cout << "error in getting addrinfo  \n";
        return 1;
    }
    int sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (sockfd<-1){
        std::cout << "error in sockfd\n";
        freeaddrinfo(res);
        return 1;
    }
    int conn=connect(sockfd,res->ai_addr,res->ai_addrlen);
    if (conn<0){
        perror("connect");
        std::cout << "error in connect\n";
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);
    std::cout << "connected to server  \n" ;
    int yes=1;
    std::string get_http="GET "+path+" HTTP/1.1\r\nHost: "+ host + "\r\nConnection: close\r\n\r\n"; 
    int sen=send(sockfd,get_http.c_str(),strlen(get_http.c_str()),0);
    char buff[1024];
    size_t rec;
    while(rec<=-1){
    rec=recv(sockfd,buff,sizeof(buff)-1,0);
    if (rec>0){
        buff[rec]='\0';
        std::cout << "received " << buff;
    }}
    close(sockfd);
    return 1;
}
