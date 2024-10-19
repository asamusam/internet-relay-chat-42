#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include "ircserv.hpp"
#include "connection.hpp"
#include "internal_error.hpp"

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
		{
			std::cerr << "Error while waiting for events (epoll_wait())" << std::strerror(errno) << "\n";
			throw (-1);
		}

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
	{
		std::cerr << "Must have two arguments: listening port and server password\n";
		return (1);
	}

	try
	{
		conn_loop(parse_port(argv[1]));
	}
	catch (int e)
	{
		if (e == -1)
		{
			std::cerr << "Program will exit with failure due to a system call error\n";
			return (2);
		}
		if (e > 0)
		{
			std::cerr << "Internal Error: " << InternalError::get_error_message((internal_error_code) e) << "\n";
			return (1);
		}
	}

    return (0);
}
