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
    serv_addr.sin_port = htons(9119);

    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
        perror("setsockopt");
    }

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("\n Error : Could not bind socket \n");
        perror("Bind");
        return 1;
    }
    if (listen(listenfd, 10) < 0){
        printf("\n Error : Listen Error \n");
        perror("Listen");
        return 1;
    }
    if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0){
        printf("\n Error : Accept Error \n");
        perror("Accept");
        return 1;   
    }
    close(listenfd);
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

    sendBuff[0] = '\r';
    strncpy(sendBuff + 1, name, strlen(name));
    char* msgStart = sendBuff + 1 + strlen(name);
    msgStart[0] = ':';
    msgStart[1] = ' ';
    msgStart += 2;
    int maxLen = sizeof(sendBuff) - strlen(name) - 4;


    fd_set read_set, ready_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(connfd, &read_set);     


    while (1) {
        ready_set = read_set;
        select(connfd+1, &ready_set, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &ready_set)){
            memset(msgStart, '\0', maxLen);
            if (fgets(msgStart, maxLen-3, stdin) != NULL){
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
                write(STDOUT_FILENO, msgEnd, strlen(msgEnd));
                write(connfd, sendBuff, strlen(sendBuff));
            }
            
        }
        if (FD_ISSET(connfd, &ready_set)) {
            count = read(connfd, recvBuff, sizeof(recvBuff) - 1);
            recvBuff[count] = '\0';
            if(count > 0)
            {
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

