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
static void child_handler(int sig);

int main(int argc, char *argv[])
{
    char name[31] = {'\0'};
    printf("What is your name?\n > ");
    fgets(name, 30, stdin);
    strtok(name, "\n");
    printf("\n");

    printf("Which IP would you like to connect to, %s?\n > ", name);

    char IP[101] = {'\0'};
    fgets(IP, 100, stdin);
    strtok(IP, "\n");
    printf("\n");

    printf("Connecting...\r");
    fflush(stdout);

    int connfd;
    if ((connfd = connect_server(IP, 9119)) < 0)
    {
        printf("Error : Could Not Connect To Server\n");
        return 1;
    }

    write(connfd, name, strlen(name));

    char recvBuff[1024] = {'\0'};
    char sendBuff[1024] = {'\0'};

    int count = read(connfd, recvBuff, sizeof(recvBuff)-1);
    recvBuff[count] = '\0';
    char partnerName[31] = {'\0'};
    strncpy(partnerName, recvBuff, count);
    printf("Connection Successful! - You are now chatting with: %s\n\n", partnerName);
    printf("Type '-quit' at any point to sign off\n\n > ");

    pid_t child_pid;
    if ((child_pid = fork()) == 0){
        while (1)
        {
            count = read(connfd, recvBuff, sizeof(recvBuff) - 1);
            recvBuff[count] = '\0';
            if(count > 0)
            {
                if (strncmp(recvBuff, "-quit", 5) == 0){
                    printf("\r        ");
                    fflush(stdout);
                    return 0;
                }
                write(1, recvBuff, strlen(recvBuff));
            }
        } 
    }
    else{
        signal(SIGCHLD, child_handler);

        sendBuff[0] = '\r';
        strncpy(sendBuff + 1, name, strlen(name));
        char* msgStart = sendBuff + 1 + strlen(name);
        msgStart[0] = ':';
        msgStart[1] = ' ';
        msgStart += 2;
        int maxLen = sizeof(sendBuff) - strlen(name) - 4;
        while (1)
        {
            memset(msgStart, '\0', maxLen);
            if (fgets(msgStart, maxLen-3, stdin) != NULL){
                if (strncmp(msgStart, "-quit", 5) == 0){
                    printf("\r\t\n\tYou have signed off - Goodbye!\n");
                    fflush(stdout);
                    write(connfd, msgStart, strlen(msgStart));
                    close(1);
                    kill(child_pid, SIGKILL);
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
    }

}

static void child_handler(int sig)
{
    waitpid(-1, NULL, WNOHANG);
    printf("\r\tPartner has signed off - Goodbye!\n");
    exit(0);
}


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

