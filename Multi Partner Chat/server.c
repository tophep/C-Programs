#include "server.h"


int main(int argc, char *argv[])
{

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

        // Create listening socket for incoming connections
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket\n");
        return 1;
    } 
    

    memset(&serv_addr, '\0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;

        // Uses port 9119
    serv_addr.sin_port = htons(PORT_NUM);

    int optval = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0){
        perror("setsockopt");
    }

        // Bind the listening socket to the hosts IP and the chosen Port
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Bind");
        return 1;
    }

        // Listen for connection requests
    if (listen(listenfd, 10) < 0){
        perror("Listen");
        return 1;
    }

    clientpool pool;
    init_pool(listenfd, &pool);
    pid_t child_pid;
    int waiting = 0;

    while (1) {
        if (pool.num_clients == 0){
            waiting = 1;
                // Fork a child process to print waiting message, child will never exit 
                // the waitingText() method and must be killed by the parent
            if ((child_pid = fork()) == 0)
                waitingText(); 
        }
            // Block until there is a new connection request or message
        pool.ready_set = pool.read_set;
        int num_ready = select(pool.max_fd+1, &pool.ready_set, NULL, NULL, NULL);

            // New connection request
        if (FD_ISSET(listenfd, &pool.ready_set)) {
                // Kill the childing printing the waiting message
            if (waiting) {
                waiting = 0;
                kill(child_pid, SIGKILL);
                    // Clear out the waiting message
                printf("\r                                                  \r\n");
            }
                
            if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) < 0){
                perror("Accept");  
            }
            else {
                add_client(connfd, &pool);
            }
        }
            // Check for new messages from connected clients
        check_clients(&pool, num_ready);
    }
}

void init_pool(int listenfd, clientpool *pool){
    pool->max_fd = listenfd;
    pool->max_index = -1;
    pool->num_clients = 0;
    FD_ZERO(&pool->read_set);
    FD_ZERO(&pool->ready_set);
    for (int i=0; i< FD_SETSIZE; i++) 
        pool->active_fds[i] = -1;

    FD_SET(listenfd, &pool->read_set);
}


void waitingText() {
    printf("No one is in the chat room!\n\nListening For Connections on Port %d", PORT_NUM);
    fflush(stdout);
    while(1){
        for (int i = 0; i < 9; i++) {
            sleep(1);
            printf(".");
            fflush(stdout);
        }
        for (int i = 0; i < 9; i++) {
            sleep(1);
            printf("\b \b");
            fflush(stdout);
        }
    }
}

void add_client(int clientfd, clientpool *pool){
    int i = 0;
    for (; i < FD_SETSIZE; i++)
    {
            // Empty slots will contain -1
        if (pool->active_fds[i] < 0){
            pool->active_fds[i] = clientfd;
            pool->num_clients++;
            FD_SET(clientfd, &pool->read_set);
            if (clientfd > pool->max_fd)
                pool->max_fd = clientfd;
            if (i > pool->max_index)
                pool->max_index = i;
            break;
        }
    }
        // Close the connection if the chat is too full
    if (i == FD_SETSIZE){
        close(clientfd);
    }
    else {
        char intro[100] = {'\0'};
        int count = read(clientfd, intro, sizeof(intro) - 1);
            // Create and add new clients name to name array
        char *name = malloc(sizeof(char) * (count + 1));
        strcpy(name, intro);
        pool->client_names[i] = name;

            // Tell other clients a new client has joined
        strcpy(&intro[count], " has joined the chat!\n\n");
        printf("%s", intro);
        write_message(clientfd, intro, pool);
    }
    
}

void remove_client(int index, clientpool *pool){
        // Sanity check out of bounds
    if (index >= 0 && index <= pool->max_index){
            // Remove client data from the pool
        int clientfd = pool->active_fds[index];
        pool->active_fds[index] = -1;
        pool->num_clients--;
        FD_CLR(clientfd, &pool->read_set);
        close(clientfd);

        char outro[100] = {'\0'};
        char *name = pool->client_names[index];
        strcpy(outro, name);
        strcpy(&outro[strlen(name)], " has left the chat!\n\n");

        free(name);
        pool->client_names[index] = NULL;

            // Tell other clients a client has left
        printf("%s", outro);
        write_message(clientfd, outro, pool);
    }
}

void write_message(int clientfd, char *clientmsg, clientpool *pool){
    for (int i = 0; i <= pool->max_index; i++){
        int recvfd = pool->active_fds[i];
        if (recvfd != -1 && recvfd != clientfd){
            write(recvfd, clientmsg, strlen(clientmsg));
        }
    }
}

void check_clients(clientpool *pool, int num_ready){
    for (int i = 0; i <= pool->max_index && num_ready > 0; i++){
        int clientfd = pool->active_fds[i];
            // Check fd is valid and waiting to be read from
        if (clientfd != -1 && FD_ISSET(clientfd, &pool->ready_set)){
            num_ready--;
            char recvbuf[2048];
            int count = read(clientfd, recvbuf, sizeof(recvbuf) - 1);
            recvbuf[count] = '\0';
            if (count == 0 || strncmp(recvbuf, "-quit", 5) == 0){
                remove_client(i, pool);
            }
            else if (count > 0){
                write_message(clientfd, recvbuf, pool);
            }
        }
    }
}

