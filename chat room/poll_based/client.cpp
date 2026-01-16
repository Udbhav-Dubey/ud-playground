#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <vector>
#include <poll.h>
bool set_nonblocking(int fd){
    int flags=fcntl(fd,F_GETFL,0);
    if (flags==-1){
        perror("fnctl FGETFL");
    }
    if (fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1){
        perror("fnctl F_SETFL");
        return false;
    }
    return true;
    }
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
    freeaddrinfo(res);
    std::cout << "[+] connected to server: \n";
    if (!set_nonblocking(sockfd)){
        close(sockfd);
        return 1;
    }
    if (!set_nonblocking(STDIN_FILENO)){
        close(sockfd);
        return 1;
    }
    std::vector<pollfd>poll_fds;
    poll_fds.push_back({
        .fd=STDIN_FILENO,
        .events=POLLIN,
        .revents=0
            });
    poll_fds.push_back({
        .fd=sockfd,
        .events=POLLIN,
        .revents=0
            });
    std::string read_buffer;
    std::string serv_buffer;
    std::string writebuf;
    std::string curr_line;
    size_t BUF_SIZE=1024;
    std::cout << "type quit to exit\n";
    std::cout << "> " << std::flush ;
    while(true){
        int ready = poll(poll_fds.data(),poll_fds.size(),-1);
        if (ready<0){
            perror("poll");
            break;
        }
        if (poll_fds[0].revents&POLLIN){
            char buf[BUF_SIZE];
            int rr=read(STDIN_FILENO,buf,BUF_SIZE);
            if (rr<0){
                perror("read stdin");
                break;
            }
            else if (rr==0){
                perror("\n[+] bye bye \n");
                break;
            }
            else {
                read_buffer.append(buf,rr);
                if (read_buffer=="quit"){
                    break;
                }
                else {
                    writebuf+=read_buffer+"\n";
                    poll_fds[1].events|=POLLOUT;
                    std::cout << "> " << std::flush;
                    read_buffer.clear();
                }
            }
        }
        if (poll_fds[1].revents&POLLIN){
            char buf[BUF_SIZE];
            int rr=recv(sockfd,buf,BUF_SIZE,0);
            if (rr<0){
                perror("recv");
                break;
            }
            if (rr==0){
                std::cout << "server said bye bye\n";
                break;
            }
            else {
                serv_buffer.append(buf,rr);
                std::cout << serv_buffer << " >" << std::flush;
                serv_buffer.clear();
            }
        }
        if (poll_fds[1].revents&POLLOUT){
            if (!writebuf.empty()){
                int s=send(sockfd,writebuf.data(),writebuf.size(),0);
                if (s<0){
                    perror("send");
                    break;
                }
                else {
                    writebuf.erase(0,s);
                    if (writebuf.empty()){
                        poll_fds[1].events&=~POLLOUT;
                    }
                }
            }
        }
        if(poll_fds[1].revents&POLLHUP){
            std::cout << "\n[!] connection error \n";
            break;
        }
    }
    close(sockfd);
    int flags=fcntl(STDIN_FILENO,F_GETFL,0);
    fcntl(STDIN_FILENO,F_SETFL,flags&~O_NONBLOCK);
    return 0;
}
