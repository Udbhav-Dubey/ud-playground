#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>
#include <string>
#define BUFFER_SIZE 1024
#define BACKLOG 10

int main (){  
    struct addrinfo hints{},*res,*p;
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    int yes=1;
    int status=getaddrinfo(NULL,"3031",&hints,&res);
    if (status!=0){
        std::cerr<<"error in getaddrinfo" << gai_strerror(status)<<"\n";
        return 1;
    }
    int sockfd;
    for (p=res;res!=nullptr;p=p->ai_next){
        sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
        if (sockfd==-1){
            perror("error in socket ");
            continue;
        }

        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
            perror("setsockopt problem");
            freeaddrinfo(res);
            return 2;
        }
        if (bind(sockfd,p->ai_addr,p->ai_addrlen)==-1){
            perror("bind");
            close(sockfd);
            continue;
        }
            break;
    }
    if (p==nullptr){
        std::cerr<<"failed to bind to any address\n";
        freeaddrinfo(res);
        return 3;
    }
    freeaddrinfo(res);
    std::cout <<"[+] bound successfully on port " << "3031" << "\n";
    if (listen(sockfd,BACKLOG)==-1){
        perror("listen");
        close (sockfd);
        return 4;
    }
    std::cout << "[+] listening for incoming connections \n";

    struct sockaddr_storage their_addr;
    socklen_t addr_size=sizeof their_addr;
    int new_sd=accept(sockfd,(struct sockaddr*)&their_addr,&addr_size);
    if (new_sd==-1){
        perror("accept eror line 61 \n");
        close (sockfd);
        return 5;
    }
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;
    if (their_addr.ss_family=AF_INET){
        struct sockaddr_in*ipv4=(struct sockaddr_in*)&their_addr;
        addr=&(ipv4->sin_addr);
    }else {
        struct sockaddr_in6*ipv6=(struct sockaddr_in6*)&their_addr;
        addr=&(ipv6->sin6_addr);
    }
    inet_ntop(their_addr.ss_family,addr,ipstr,sizeof ipstr);
    std::cout << "[+] connection secured from " << ipstr << "\n";

    char buffer[BUFFER_SIZE];
    std::string message;
    while(true){
        memset(buffer,0,BUFFER_SIZE);
        int bytes=recv(new_sd,buffer,BUFFER_SIZE-1,0);
        if (bytes<=0){
            if (bytes==0){
                std::cout << "[+] client disconnected\n";
            }
            else {
                perror("problem in recv end ");
            }
            break;
        }
        buffer[bytes]='\0';
        std::cout << "Client : " << buffer ;
        if (strncmp(buffer,"quit",4)==0){
            std::cout << "client closed the connection ";
            break;
        }
        std::cout << "server : " ;
        std::getline(std::cin,message);
        message+='\n';
        if (send(new_sd,message.c_str(),message.length(),0)==-1){
            perror("send");
            break;
        }
        if (message.find("quit")==0){
            std::cout <<"[+]server closed the connection \n";
            break;
        }
    }
    close(new_sd);
    close(sockfd);
    std::cout << "[+] server shut down\n";
    return 0;
}
