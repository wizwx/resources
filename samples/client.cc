#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// refer to https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/client.c
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
//
// https://www.gnu.org/software/libc/manual/html_node/Byte-Stream-Example.html
// https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html

int main(int argc, char *argv[])
{
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    struct hostent * server = gethostbyname(argv[1]);
    if (server == nullptr) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));      // argv[2] is port #
    serv_addr.sin_addr = *(struct in_addr *)server->h_addr;

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    char buffer[256];
    bzero(buffer,256);

    printf("Please enter the message: ");
    fgets(buffer,255,stdin);
    int n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) error("ERROR writing to socket");

    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    printf("%s\n",buffer);
    return 0;
}
