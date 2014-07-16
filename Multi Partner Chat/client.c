#include "client.h"


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

    int connfd;

        // Port 9119 is assumed and used in the server code
    if ((connfd = connect_server(IP, PORT_NUM)) < 0)
    {
        printf("Error : Could Not Connect To Server\n");
        return 1;
    }

        // Send user's name over the connection
    write(connfd, name, strlen(name));

    char recvBuff[1024] = {'\0'};
    char sendBuff[1024] = {'\0'};

    printf("You have successfully connected to the chatroom!\n\n");
    printf("Type '-quit' at any point to sign off\n\n > ");
    fflush(stdout);

        // Add prefix to the send buffer so that all messages are sent with the user's name
    strncpy(sendBuff, name, strlen(name));
    char* msgStart = sendBuff + strlen(name);
    msgStart[0] = ':';
    msgStart[1] = ' ';
    msgStart += 2;
        // Max message size must account for name + ": " and "\n\0" at the end
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
            if (fgets(msgStart, maxLen, stdin) != NULL){
                    
                    // If user sends quit message tell partner and exit 
                if (strncmp(msgStart, "-quit", 5) == 0){
                    printf("\r\t\n\n\tYou have signed off - Goodbye!\n");
                    write(connfd, msgStart, strlen(msgStart));
                    return 0;
                }
                char* msgEnd = sendBuff + strlen(sendBuff);
                msgEnd[0] = '\n';
                write(STDOUT_FILENO, "\n > ", 4);
                write(connfd, sendBuff, strlen(sendBuff));
            }
            
        }

            // If connfd is ready print message to the terminal
        if (FD_ISSET(connfd, &ready_set)) {
            int count = read(connfd, recvBuff, sizeof(recvBuff) - 1);
            recvBuff[count] = '\0';
                // Clear out the prompt line
            printf("\r                                                  \r");
            fflush(stdout);
            if(count > 0)
            {
                write(STDOUT_FILENO, recvBuff, strlen(recvBuff));
                write(STDOUT_FILENO, " > ", 3);
            }
            else if (count == 0){
                printf("Sorry - The server has disconnected or the chatroom was full!\n");
                return 0;
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






