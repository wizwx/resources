#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// refer to https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/server.html
//
// https://www.gnu.org/software/libc/manual/html_node/Server-Example.html
// https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html

int main(int argc, char *argv[])
{

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) error("ERROR opening socket");

     struct sockaddr_in serv_addr;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_port = htons(atoi(argv[1])); // atoi(argv[1]) is port #
     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     // serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
              error("ERROR on bind");

     if (listen(sockfd,1) < 0) error("ERROR on listen");

     struct sockaddr_in cli_addr;
     socklen_t clilen = sizeof(cli_addr);
     int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) error("ERROR on accept");
     fprintf(stderr, "Server: connect from host %s, port %hd.\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

     char buffer[256];
     bzero(buffer,256);

     int n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");

     return 0; 
}
