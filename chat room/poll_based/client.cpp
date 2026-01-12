#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <netdb.h>
#define buffer_size 1024
int main (int argc,char*argv[3]){
    if (argc!=3){
        std::cout << "error: not enough inputs please type ip then port with it also\n";
        return 1;
    }
    const char *hostname=argv[1];
    const char *port=argv[2];
    struct addrinfo hints,*res,*p;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    int status=getaddrinfo(hostname,port,&hints,&res);
    if (status!=0){
        std::cout<< "error in status : " << gai_strerror(status) << "\n"; 
        return 1;
    }
    int sockfd;
    for (p=res;p!=nullptr;p=p->ai_next){
        sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
        if (sockfd<0){
            perror("sockfd");
            continue;
        }
        if (connect(sockfd,p->ai_addr,p->ai_addrlen)==-1){
            perror("connect");
            close(sockfd);
            continue;
        }
        break;
    }
    if (p==nullptr){
        std::cout << "unable to connect\n";
        freeaddrinfo(res);
        return 1;
    }
    char buffer[buffer_size];
    std::string message;
    std::cout << "type quit to leave : \n";
    while(true){
        std::cout << "Client : " ;
        std::getline(std::cin,message);
        message+='\0';
        if (send(sockfd,message.c_str(),message.size(),0)==-1){
            perror("send");
            break;
        }
        if (message.find("quit")==0){
            std::cout << "closing connection\n";
            break;
        }
        memset(buffer,0,sizeof(buffer));
        int bytes=recv(sockfd,buffer,buffer_size-1,0);
        if (bytes<0){
            perror("bytes");
            break;
        }
        if (bytes==0){
            std::cout << "connection closed by server\n";
            break;
        }
        buffer[bytes]='\0';
        std::cout << "Server : " << buffer << "\n";
        if (strncmp(buffer,"quit",4)==0){
            std::cout << "connection closed by client\n";
            break;
        }
    }
    close(sockfd);
    return 0;
}
