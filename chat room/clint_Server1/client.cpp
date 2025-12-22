#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
int main (){
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if (sock<0){
        perror("socket");
        return 1;
    }
    sockaddr_in server_addr{};
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(8081);

    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);
    if (connect(sock,(sockaddr*)&server_addr,sizeof(server_addr))<0){
        perror("connect");
        close(sock);
        return 1;
    }
    std::cout << "[+] connected to server\n";
    char buffer[1024];
    int bytes=recv(sock,buffer,sizeof(buffer)-1,0);
    if (bytes>0){
        buffer[bytes]='\0';
        std::cout << "[server]" << buffer << "\n";
    }
    const char *msg="Hello sever , I got your message !";
    send(sock,msg,strlen(msg),0);
    close(sock);
    std::cout << "[+] Disconnected\n";
    return 0;
}
