#include <cstring>
#include <sys/epoll.h>
#include "ircserv.hpp"
#include "connection.hpp"

void conn_loop(int port)
{
	int nfds = 0;
	struct epoll_event events[ConnConst::max_events];
	(void) std::memset(events, 0, sizeof(events));

	int listen_sock_fd = listen_sock_init(port);
	int epoll_fd = epoll_init(listen_sock_fd);

	for (;;)
	{
		nfds = epoll_wait(epoll_fd, events, ConnConst::max_events, ConnConst::time_out_ms);
		if (-1 == nfds)
			throw (-1); // TODO: better error (this is a syscall error)

		for (int i = 0; i < nfds; ++i)
		{
			if (events[i].data.fd == listen_sock_fd)
				accept_in_conns(epoll_fd, listen_sock_fd);
			else
			{
				// TODO: deal with the data;
			}
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 3)
		return (1); // TODO: better error

	try
	{
		conn_loop(parse_port(argv[1]));
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
