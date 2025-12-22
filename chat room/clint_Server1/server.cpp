#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
int main (){
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    if (server_fd==-1){
        perror("socket");
        return -1;
    }
    std::cout << "[+]Socket created\n";
    sockaddr_in server_addr{};
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(8081);
    server_addr.sin_addr.s_addr=INADDR_ANY;

    if (bind(server_fd,(sockaddr*)&server_addr,sizeof(server_addr))<0){
        perror("bind");
        close(server_fd);
        return 1;
    }
    std::cout << "[+] bound to port 8080\n";
    if (listen(server_fd,5)<0){
        perror("listen");
        close(server_fd);
        return 1;
    }
    std::cout << "[+] listing for incoming connections...\n";

    sockaddr_in client_addr{};
    socklen_t client_len=sizeof(client_addr);
    int client_fd=accept(server_fd,(sockaddr*)&client_addr,&client_len);
    if (client_fd<0){
        perror("accept");
        close(server_fd);
        return 1;
    }
    std::cout << "[+] client connected from " << inet_ntoa(client_addr.sin_addr)
        << ":" << ntohs(client_addr.sin_port)<<"\n";
    const char* msg="Hello from server !\n";
    send(client_fd,msg,strlen(msg),0);
    char buffer[1024];
    int bytes = recv(client_fd,buffer,sizeof(buffer)-1,0);
    if (bytes>0){
        buffer[bytes]='\0';
        std::cout << "[client]" << buffer << "\n";
    }
    close(client_fd);
    close(server_fd);
    std::cout << "[+] connection closed.\n";
    return 0;
}
