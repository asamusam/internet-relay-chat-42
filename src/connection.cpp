#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "connection.hpp"

static const int g_max_events = 12;
static const int g_max_conns = 12;
static const int g_time_out_ms = NO_TIMEOUT;

int parse_port(char *s)
{
	int port = 0;

	std::string cpps(s);
	std::istringstream iss(cpps);
	iss >> port;

	if (port < 1024 || port > std::numeric_limits<unsigned short>::max())
		throw (1); // TODO: better error

	return (port);
}

int listen_sock_init(int port)
{
	int sock_fd = -1;
	struct sockaddr_in sai;
	(void) std::memset(&sai, 0, sizeof(sai));

	sai.sin_family = AF_INET;
	sai.sin_port = htons(port);
	sai.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	sock_fd = socket(sai.sin_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (-1 == sock_fd)
		throw (-1); // TODO: better error message

	if (-1 == bind(sock_fd, (struct sockaddr *) &sai, sizeof(sai)))
		throw (-1); // TODO: better error message

	if (-1 == listen(sock_fd, g_max_conns))
		throw (-1); // TODO: better error message

	return (sock_fd);
}

int epoll_init(int listen_sock_fd)
{
	int ep_fd = -1;
	epoll_event ev;
	(void) std::memset(&ev, 0, sizeof(ev));

	ep_fd = epoll_create1(0);
	if (-1 == ep_fd)
		throw (-1); // TODO: better error message

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock_fd;
	if (-1 == epoll_ctl(ep_fd, EPOLL_CTL_ADD, listen_sock_fd, &ev))
		throw (-1); // TODO: better error message

	return (ep_fd);
}

static void accept_in_conns(int epoll_fd, int listen_sock_fd)
{
	int conn_sock_fd = -1;
	epoll_event ev;
	(void) std::memset(&ev, 0, sizeof(ev));

	conn_sock_fd = accept4(listen_sock_fd, NULL, NULL, SOCK_NONBLOCK);	
	if (-1 == conn_sock_fd)
		throw (-1); // TODO: better error (this is a syscall error)

	ev.events = EPOLLIN;
	ev.data.fd = conn_sock_fd;
	if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock_fd, &ev))
		throw (-1); // TODO: better error (this is a syscall error)
}

void conn_loop(int epoll_fd, int listen_sock_fd)
{
	int nfds = 0;
	struct epoll_event events[g_max_events];
	(void) std::memset(events, 0, sizeof(events));

	for (;;)
	{
		nfds = epoll_wait(epoll_fd, events, g_max_events, g_time_out_ms);
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
