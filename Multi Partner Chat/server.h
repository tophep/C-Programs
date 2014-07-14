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

typedef struct clientpool{ 
    int max_fd;
    int max_index;
    int num_clients;
    int active_fds[FD_SETSIZE];
    char *client_names[FD_SETSIZE];
    fd_set read_set;
    fd_set ready_set;
} clientpool;

#define PORT_NUM 9119


void init_pool(int listenfd, clientpool *pool);
void waitingText();
void add_client(int client_fd, clientpool *pool);
void remove_client(int index, clientpool *pool);
void write_message(int clientfd, char *clientmsg, clientpool *pool);
void check_clients(clientpool *pool, int num_ready);
