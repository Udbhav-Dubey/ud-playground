#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <cstring>
#include <poll.h>
#include <algorithm>
#include <unordered_map>
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
    sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
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
std::vector<pollfd> poll_fds;
poll_fds.push_back({
   .fd=sockfd,
   .events=POLLIN,
   .revents=0
});
size_t BUF_SIZE=1024;
int timeout_millsecs=500;
std::unordered_map<int,std::pair<std::string,std::string>> map;
std::string messeze;
map[0]={"server socket","whats your name da \n"};
while(true){
    int red=poll(poll_fds.data(),poll_fds.size(),timeout_millsecs);
    if (red<0){
        if (errno==EINTR){continue;}
        perror("poll");
        break;
    }
    for (int i=0;i<poll_fds.size();i++){
        pollfd &pfd=poll_fds[i];
        if ((pfd.fd==sockfd)&&(pfd.revents&POLLIN)){
            sockaddr_storage addr;
            socklen_t alen=sizeof(addr);
            int accfd=accept(sockfd,(sockaddr*)&addr, &alen);
            if (accfd<0){
                perror("accept");
                continue;
            }
            char wbuf[1024];
            //int wastrecv=recv(accfd,wbuf,1024,0);

            int ss=send(accfd,map[0].second.data(),map[0].second.size(),0);
            if (ss<0){
                std::cout << "couldnt send name messege \n";
                perror("send");
            }
            char cl_name[256];
            recv(accfd,cl_name,256,0);
            map[accfd]={cl_name," "};
            poll_fds.push_back({.fd=accfd,.events=POLLIN,.revents=0});
            std::cout << "New client : fd = " << accfd << "\n";
        }
        if (pfd.revents&POLLIN){
            std::cout << map[pfd.fd].first << " is saying something\n";
            char buf[BUF_SIZE];
            int rec=recv(pfd.fd,buf,BUF_SIZE,0);
            if (rec==0){
                close(pfd.fd);
                poll_fds.erase( 
                std::remove_if(poll_fds.begin(),poll_fds.end(),[&](const pollfd&x){return x.fd==pfd.fd;}),
                    poll_fds.end()        
                );
                continue;
            }
            if (rec<0){
                perror("recv");
                continue;
            }
            // need to clear messeze also 
           // messeze.append(buf,rec);
            map[pfd.fd].second.append(buf,rec);
            pfd.events|=POLLOUT;
           /* for (int j=0;j<poll_fds.size();j++){
                if (pfd.fd==poll_fds[j].fd){
                    continue;
                }
                else {
                    poll_fds[j].events|=POLLOUT;
                }
            }
            */
        }
        if (pfd.revents&POLLOUT){
            for (int j=0;i<poll_fds.size();j++){
                if (pfd.fd==poll_fds[j].fd){
                    continue;
                }
                else {
            std::string tbs=map[pfd.fd].first+" : "+map[pfd.fd].second;
            int senty=send(poll_fds[j].fd,tbs.data(),tbs.size(),0);
            if (senty<0){
            perror("send");
            continue;
            }
            tbs.clear();
            map[pfd.fd].second.clear();
                }
            }
            }
        if (pfd.revents&POLLHUP){
            close(pfd.fd);
            poll_fds.erase(
            std::remove_if(poll_fds.begin(),poll_fds.end(),[&](const pollfd&x){return x.fd==pfd.fd;}),poll_fds.end()
                    );
            // remove map too
            break;
        }
    }
}
close(sockfd);
return 0;
}
