#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <unistd.h>
#define BUFFER_SIZE 1024

int main (int argc,char *argv[3]){
    if (argc!=3){
        std::cerr<<"error in main only not enough inputs " << argv[0] << "<hostname><port>\n";
        return 1;
    }
    const char*hostname=argv[1];
    const char*port=argv[2];
    struct addrinfo hints{},*res,*p;
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    
    int status =getaddrinfo(hostname,port,&hints,&res);
    if (status!=0){
        std::cerr<<"problem in status only " << gai_strerror(status) <<"\n";
        return 2;
    }
    int sockfd;
    for (p=res;p!=nullptr;p=p->ai_next){
        sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
        if (sockfd==-1){
            perror("problem in sockfd");
            close (sockfd);
            continue;
        }
        if (connect(sockfd,p->ai_addr,p->ai_addrlen)==-1){
            perror("problem in connect");
            close(sockfd);
            continue;
        }
        break;
    }
    if (p==nullptr){
        std::cerr<<"failed to connect \n";
        freeaddrinfo(res);
        return 3;
    }
    std::cout << "[+] connected to " << hostname << "on port : " << port << "\n";
    freeaddrinfo(res);

    char buffer[BUFFER_SIZE];
    std::string message;
    std::cout << "type quit to leave \n";
    while(true){
        std::cout << "Client : ";
        std::getline(std::cin,message);
        message+='\n';
        if (send(sockfd,message.c_str(),message.length(),0)==-1){
            perror("send");
            break;
        }
        if (message.find("quit")==0){
            std::cout << "[+] closing connection\n";
            break;
        }
    memset(buffer,0,BUFFER_SIZE);
    int bytes_recieved=recv(sockfd,buffer,BUFFER_SIZE-1,0);
    if (bytes_recieved<=0){
        if (bytes_recieved==0){
            std::cout <<"server disconnected\n";
        }else {
            perror("recv is the error");
        }
        break;
    }
    buffer[bytes_recieved]='\0';
    std::cout << "Server : " << buffer ;
    if (strncmp(buffer,"quit",4)==0){
        std::cout <<"[+] server closed the connection\n";
        break;
    }
    }
    close(sockfd);
    return 0;
}
