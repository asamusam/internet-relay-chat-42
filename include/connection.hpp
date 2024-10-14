#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#define NO_TIMEOUT -1
#define MAX_MSG_SIZE 512

int parse_port(char *s);
int listen_sock_init(int port);
int epoll_init(int listen_sock_fd);
void conn_loop(int epoll_fd, int listen_sock_fd);

#endif /* CONNECTION_HPP */
