#include <cerrno>
#include <cstring>
#include <iostream>
#include <limits>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include "InternalError.hpp"
#include "SystemCallErrorMessage.hpp"
#include "connection.hpp"

int parse_port(char *s)
{
	int port = 0;

	std::string cpps(s);
	std::istringstream iss(cpps);
	iss >> port;

	if (port < 1024 || port > std::numeric_limits<unsigned short>::max())
		throw (IEC_BADPORTNUM);

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
		throw (SCEM_SOCKET);

	if (-1 == bind(sock_fd, (struct sockaddr *) &sai, sizeof(sai)))
		throw (SCEM_BIND);

	if (-1 == listen(sock_fd, ConnConst::max_conns))
		throw (SCEM_LISTEN);

	return (sock_fd);
}

int epoll_init(int listen_sock_fd)
{
	int ep_fd = -1;
	epoll_event ev;
	(void) std::memset(&ev, 0, sizeof(ev));

	ep_fd = epoll_create1(0);
	if (-1 == ep_fd)
		throw (SCEM_EPOLL_CREATE);

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock_fd;
	if (-1 == epoll_ctl(ep_fd, EPOLL_CTL_ADD, listen_sock_fd, &ev))
		throw (SCEM_EPOLL_CTL);

	return (ep_fd);
}

void accept_in_conns(int epoll_fd, int listen_sock_fd)
{
	int conn_sock_fd = -1;
	epoll_event ev;
	(void) std::memset(&ev, 0, sizeof(ev));

	conn_sock_fd = accept4(listen_sock_fd, NULL, NULL, SOCK_NONBLOCK);	
	if (-1 == conn_sock_fd)
	{
		std::cerr << "Error while trying to accept outside connection (accept4()): " << std::strerror(errno) << "\n";
		return ;
		// TODO: Handle error approprately ? should be handled by conn_loop();
	}

	ev.events = EPOLLIN;
	ev.data.fd = conn_sock_fd;
	if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock_fd, &ev))
	{
		std::cerr << "Error while trying to accept outside connection (accept4()): " << std::strerror(errno) << "\n";
		return ;
		// TODO: Handle error approprately ? should be handled by conn_loop();
	}
}
