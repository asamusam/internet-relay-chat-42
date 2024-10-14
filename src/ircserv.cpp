#include "ircserv.hpp"
#include "connection.hpp"

int main(int argc, char **argv)
{
	if (argc != 3)
		return (1); // TODO: better error

	try
	{
		int listen_sock_fd = listen_sock_init(parse_port(argv[1]));
		int epoll_fd = epoll_init(listen_sock_fd);
		conn_loop(epoll_fd, listen_sock_fd);
	}
	catch (int e)
	{
		if (e == -1)
			return (1); // TODO: its a syscall error, handle appropriatley
		if (e == 1)
			return (1); // TODO: its another type of error, handle appropriatley
	}

    return 0;
}
