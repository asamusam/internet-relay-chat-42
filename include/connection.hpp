#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#define NO_TIMEOUT -1
#define MAX_MSG_SIZE 512

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
void accept_in_conns(int epoll_fd, int listen_sock_fd);

#endif /* CONNECTION_HPP */
