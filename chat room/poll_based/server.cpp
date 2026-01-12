#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <cstring>
const constexpr char* PORT ="9669";
int main (){
struct addrinfo hints,*res,*p;
memset(&hints,0,sizeof(hints));
hints.ai_family=AF_INET;
hints.ai_socktype=SOCK_STREAM;
hints.ai_flags=AI_PASSIVE;
int yes=1;
int status=getaddrinfo(NULL,PORT,&hints,&res);
if (status!=0){
    std::cerr<<"error in getaddrinfo " << gai_strerror(status) << "\n";
    return 1;
    }
int sockfd;
for (p=res;p!=nullptr;p=p->ai_next){
    sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if (sockfd==-1){
        perror("error in sockfd\n");
        continue;
    }
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1){
        perror("setsockopt\n");
        return 1;
    }
    if (bind(sockfd,p->ai_addr,p->ai_addrlen)==-1){
        close(sockfd);
        perror("bind\n");
        continue;
    }
    break;
}
if (p==nullptr){
    std::cerr<<"failed to bind to any port\n";
    return 1;
}
freeaddrinfo(res);
if (listen(sockfd,10)==-1){
    perror("listen");
    close(sockfd);
    return 1;
}
std::cout << "[+] listing on port " << PORT << "\n";
struct sockaddr_storage client_addr;
socklen_t addr_size=sizeof(client_addr);
int accfd=accept(sockfd,(struct sockaddr*)&client_addr,&addr_size);
if (accfd==-1){
    perror("accept");
    close(sockfd);
    return 1;
}
char ipstr[INET6_ADDRSTRLEN];
void *addr;
if (client_addr.ss_family==AF_INET){
    struct sockaddr_in*ipv4=(struct sockaddr_in*)&client_addr;
    addr=&(ipv4->sin_addr);
}    
    else {
    struct sockaddr_in6*ipv6=(struct sockaddr_in6*)&client_addr;
    addr=&(ipv6->sin6_addr);
    }
inet_ntop(client_addr.ss_family,addr,ipstr,sizeof(ipstr));
std::cout << "Connected to client : " << ipstr << "\n";
size_t buff_size=1024;
char buffer[1024];
std::string message;
bool quit=0;
while(true){
    memset(buffer,0,buff_size-1);
    int bytes=recv(accfd,buffer,buff_size-1,0);
    if (bytes==0){
        std::cout << "client disconnected\n";
        break;
    }
    else if (bytes<0){
        perror("recv");
        close(accfd);
        return 1;
    }
    buffer[bytes]='\0';
    std::cout << "Client : " << buffer << "\n";
    if (strncmp(buffer,"quit",4)==0){
        std::cout << "Client Disconnected\n";
        break;
    }
    std::cout << "Server : " ;
    std::getline(std::cin,message);
    message+='\0';
    if (message.find("quit")==0){
        std::cout << "[+]server closed its connection";
        message="server closed its connection , bye bye ";
        message+='\0';
        quit=1;
    }
    if (send(accfd,message.c_str(),message.length()-1,0)==-1){
        perror("send");
        break;
    }
    if (quit==1){break;}
}
close(sockfd);
close(accfd);
std::cout << "server closed its connections\n";
return 0;
}
