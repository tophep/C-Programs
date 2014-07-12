#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

int connect_server(char *IP, int port);


/**

    Single Partner Chat Client

    A chat client that connects to a chat server (see chatserverS.c)
    This version of the chat programs uses the system function "select" to maintain concurrency between user input and incoming messages.

    The program takes a user-supplied IP address to connect to the server, this will usually only work if the client
    and server are on the same network.

    Works on OSX and Linux: Use "gcc -o <executable name> chatclientS.c"

**/

int main(int argc, char *argv[])
{
    char name[31] = {'\0'};
    printf("What is your name?\n > ");
    fgets(name, 30, stdin);
    strtok(name, "\n");
    printf("\n");

        // Takes an IPv4 Address
    printf("Which IP would you like to connect to, %s?\n > ", name);

    char IP[101] = {'\0'};
    fgets(IP, 100, stdin);
    strtok(IP, "\n");
    printf("\n");

    printf("Connecting...\r");
    fflush(stdout);

    int connfd;

        // Port 9119 is assumed and used in the server code
    if ((connfd = connect_server(IP, 9119)) < 0)
    {
        printf("Error : Could Not Connect To Server\n");
        return 1;
    }

        // Send user's name over the connection
    write(connfd, name, strlen(name));

    char recvBuff[1024] = {'\0'};
    char sendBuff[1024] = {'\0'};

    int count = read(connfd, recvBuff, sizeof(recvBuff)-1);
    recvBuff[count] = '\0';
    char partnerName[31] = {'\0'};
    strncpy(partnerName, recvBuff, count);
    printf("Connection Successful! - You are now chatting with: %s\n\n", partnerName);
    printf("Type '-quit' at any point to sign off\n\n > ");
    fflush(stdout);

        // Add prefix to the send buffer so that all messages are sent with the user's name
    sendBuff[0] = '\r';
    strncpy(sendBuff + 1, name, strlen(name));
    char* msgStart = sendBuff + 1 + strlen(name);
    msgStart[0] = ':';
    msgStart[1] = ' ';
    msgStart += 2;
    int maxLen = sizeof(sendBuff) - strlen(name) - 4;

        // Set up a File Descriptor set to listen for Stdin and the connected socket
    fd_set read_set, ready_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(connfd, &read_set);     


    while (1) {
        ready_set = read_set;

            // Multiplex Stdin and connfd and listen for reads on both
        select(connfd+1, &ready_set, NULL, NULL, NULL);

            // If Stdin in is ready send message across connection
        if (FD_ISSET(STDIN_FILENO, &ready_set)){
            memset(msgStart, '\0', maxLen);
            if (fgets(msgStart, maxLen-3, stdin) != NULL){
                    
                    // If user sends quit message tell partner and exit 
                if (strncmp(msgStart, "-quit", 5) == 0){
                    printf("\r\t\n\n\tYou have signed off - Goodbye!\n");
                    fflush(stdout);
                    write(connfd, msgStart, strlen(msgStart));
                    return 0;
                }
                char* msgEnd = sendBuff + strlen(sendBuff);
                msgEnd[0] = '\n';
                msgEnd[1] = ' ';
                msgEnd[2] = '>';
                msgEnd[3] = ' ';
                write(1, msgEnd, strlen(msgEnd));
                write(connfd, sendBuff, strlen(sendBuff));
            }
            
        }

            // If connfd is ready print message to the terminal
        if (FD_ISSET(connfd, &ready_set)) {
            count = read(connfd, recvBuff, sizeof(recvBuff) - 1);
            recvBuff[count] = '\0';
            if(count > 0)
            {
                    // If chat partner sends quit message notify user and exit
                if (strncmp(recvBuff, "-quit", 5) == 0){
                    printf("\r                              \n");
                    printf("\r\t%s has signed off - Goodbye!\n", partnerName);
                    fflush(stdout);
                    return 0;
                }
                write(1, recvBuff, strlen(recvBuff));
            }
        }
    }

}

    // Creates a socket and attempts to connect it with the socket address provided
int connect_server(char *IP, int port) 
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket");
        return -1;
    }

    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    if(inet_pton(AF_INET, IP, &serveraddr.sin_addr)<=0)
    {
        perror("inet_pton");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Connect");
        return -1;
    }
    return sockfd;
}

