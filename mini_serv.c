#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct s_client {
    int     id;
    char    *msg;
}   t_client;

t_client    *clients;
fd_set      read_set, write_set, current;
int         current_id = 0, maxfd = 0;
char        *send_buf, *recv_buf;

void err(char *m) {
    write(2, m ? m : "Fatal error\n", strlen(m ? m : "Fatal error\n"));
    exit(1);
}

void    send_broadcast(int exclu) {
    for (int fd = 0; fd <= maxfd; fd++)
        if (FD_ISSET(fd, &write_set) && fd != exclu)
            send(fd, send_buf, strlen(send_buf), 0);
}

int main(int ac, char **av) {
    if (ac != 2)
        err("Wrong number of arguments\n");
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    int max_c = 1024, max_m = 1000000;
    
    clients = calloc(max_c, sizeof(t_client));
    send_buf = malloc(max_m + 64);
    recv_buf = malloc(max_m);
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (!clients || !send_buf || !recv_buf || sockfd < 0)
        err(NULL);

    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(atoi(av[1])); 
  
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 || listen(sockfd, 10) != 0)
        err(NULL);
    FD_ZERO(&current);
    FD_SET(sockfd, &current);
    maxfd = sockfd;
    while(1) {
        read_set = write_set = current;
        if (select(maxfd + 1, &read_set, &write_set, NULL, NULL) < 0)
            err(NULL);
        for(int fd = 0; fd <= maxfd; fd++) {
            if (!FD_ISSET(fd, &read_set))
                continue;
            if (fd == sockfd) {
                socklen_t len = sizeof(cli);
                connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
                if (connfd < 0) 
                    err(NULL);
                if (connfd > maxfd)
                    maxfd = connfd;
                clients[connfd].id = current_id++;
                clients[connfd].msg = calloc(max_m, 1);
                if (!clients[connfd].msg)
                    err(NULL);
                FD_SET(connfd, &current);
                sprintf(send_buf, "server: client %d just arrived\n", clients[connfd].id);
                send_broadcast(connfd); 
            }
            else {
                int ret = recv(fd, recv_buf, max_m, 0);
                if (ret <= 0) {
                    sprintf(send_buf, "server: client %d just left\n", clients[fd].id);
                    send_broadcast(fd);
                    free(clients[fd].msg);
                    clients[fd].msg = NULL;
                    FD_CLR(fd, &current);
                    close(fd);
                }
                else {
                    int j = strlen(clients[fd].msg);
                    for (int i = 0; i < ret; i++, j++) {
                        clients[fd].msg[j] = recv_buf[i];
                        if (clients[fd].msg[j] == '\n') {
                            clients[fd].msg[j] = '\0';
                            sprintf(send_buf, "client %d: %s\n", clients[fd].id, clients[fd].msg);
                            send_broadcast(fd);
                            bzero(clients[fd].msg, j + 1);
                            j = -1;
                        }
                    }
                }
            }
        }
    }
}