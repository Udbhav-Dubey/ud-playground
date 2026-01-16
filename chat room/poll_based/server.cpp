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
struct Client{
    int fd;
    bool name_flag;
    std::string name{};
    std::string readbuf{};
    std::string writebuf{};
};
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
std::unordered_map<int,Client> mp;
std::string messeze;
std::string nameda="whats your name";
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
            int sendy=send(accfd,nameda.data(),nameda.size(),0);
            if (sendy<0){
                std::cout << "error in sending name\n";
            }
            Client cl; 
            cl.name_flag=0;
            mp[accfd]=cl;
            poll_fds.push_back({.fd=accfd,.events=POLLIN,.revents=0});
            std::cout << "New client : fd = " << accfd << "\n";
        }
        if (pfd.revents&POLLIN){
            std::cout << mp[pfd.fd].name << " is saying something\n";
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

            if (mp[pfd.fd].name_flag==1){
            mp[pfd.fd].readbuf.append(buf,rec);
            mp[pfd.fd].writebuf+=mp[pfd.fd].readbuf;
            mp[pfd.fd].readbuf.clear();
            pfd.events|=POLLOUT;
            }
            else {
            mp[pfd.fd].name.append(buf,rec);
            mp[pfd.fd].name_flag=1;
            }
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

            int sendy=send(pfd.fd,mp[pfd.fd].writebuf.data(),mp[pfd.fd].writebuf.size(),0);
            if (sendy<0){
            //    close(pfd.fd);
            continue;
            }
            mp[pfd.fd].writebuf.erase(0,sendy);
            if(mp[pfd.fd].writebuf.empty()){
                pfd.events&=~POLLOUT;
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
