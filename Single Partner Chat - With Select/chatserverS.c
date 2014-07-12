#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

/**

    Single Partner Chat Server

    A chat server that connects to a single chat client (see chatclientS.c)
    This version of the chat programs uses the system function "select" to maintain concurrency between user input and incoming messages.

    Works on OSX and Linux: Use "gcc -o <executable name> chatserverS.c"
    
**/

int main(int argc, char *argv[])
{
    char name[31] = {'\0'};
    printf("What is your name?\n > ");
    fgets(name, 30, stdin);
    strtok(name, "\n");
    printf("\n");

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    printf("Listening For Connection...\r");
    fflush(stdout);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Error : Could not create socket \n");
        perror("Socket\n");
        return 1;
    } 
    

    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

        // Uses port 9119
    serv_addr.sin_port = htons(9119);

    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
        perror("setsockopt");
    }

        // Bind the listening socket to the hosts IP and the chosen Port
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\n Error : Could not bind socket \n");
        perror("Bind");
        return 1;
    }

        // Listen for connection requests
    if (listen(listenfd, 10) < 0){
        printf("\n Error : Listen Error \n");
        perror("Listen");
        return 1;
    }

        // Connect to the first request
    if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0){
        printf("\n Error : Accept Error \n");
        perror("Accept");
        return 1;   
    }

        // Stop listening for requests
    close(listenfd);

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


    // Set up a File Descriptor set to listen for Stdin and the connected socket connfd
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

