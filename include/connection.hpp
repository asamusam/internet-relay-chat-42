#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "App.hpp"
#include "Client.hpp"

#define NO_TIMEOUT -1
#define MAX_MSG_SIZE 512
#define CRLF "\r\n"

class ConnConst
{
	public:
		static const int max_events  = 12;
		static const int max_conns   = 12;
		static const int time_out_ms = NO_TIMEOUT;
};

int parse_port(char *s);
int listen_sock_init(int port);
int epoll_init(int listen_sock_fd);
void accept_in_conns(App &app, int epoll_fd, int listen_sock_fd);
void close_conn(App &app, int fd);
void handle_msg(App &app, Client *client);

#endif /* CONNECTION_HPP */
