#include <cerrno>
#include <cstring>
#include <iostream>
#include <limits>
#include <netinet/in.h>
#include <sstream>
#include <sys/epoll.h>
#include <unistd.h>
#include "App.hpp"
#include "Client.hpp"
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

void accept_in_conns(App &app, int epoll_fd, int listen_sock_fd)
{
	epoll_event ev;
	(void) std::memset(&ev, 0, sizeof(ev));

	int conn_sock_fd = accept4(listen_sock_fd, NULL, NULL, SOCK_NONBLOCK);	
	if (-1 == conn_sock_fd)
		throw (SCEM_ACCEPT4);

	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	ev.data.fd = conn_sock_fd;
	if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock_fd, &ev))
		throw (SCEM_EPOLL_CTL);

	static int uuid = 1;
	Client *client = new Client;
	client->uuid = uuid++;
	client->fd = conn_sock_fd;
	client->is_registered = false;
	client->has_valid_pwd = false;
	client->num_channels = 0;
	app.add_client(client);

	std::cout << "Accepted new connection and created new client with uuid:" << client->uuid << " and fd:"
		<< client->fd << "\n";
}

void handle_msg(App &app, Client *client)
{
	char buff[MAX_MSG_SIZE];
	std::string &msg = client->msg_buff;
	ssize_t bytes_read;
	size_t crlf_indx;
	Message message;

	do
	{
		std::memset(buff, 0, sizeof buff);
		bytes_read = recv(client->fd, buff, sizeof buff, 0);
		if (-1 ==  bytes_read)
			throw (SCEM_RECV);
		if (0 == bytes_read)
			return ;
		msg.append(buff);
		crlf_indx = msg.find(CRLF);

		if ((crlf_indx == msg.npos && msg.size() >= MAX_MSG_SIZE)
				|| (crlf_indx != msg.npos && crlf_indx > MAX_MSG_SIZE - 2))
		{
			msg.erase(MAX_MSG_SIZE);
			std::cout << "Recv msg from uuid:" << client->uuid << " ->" << msg << "\n";
			if (-1 == app.parse_message(*client, msg, message))
				std::cerr << "Cannot parse message from uuid:" << client->uuid << " ->" << msg << "\n";
			else
			{
				std::cout << "Exec msg from uuid:" << client->uuid << " ->" << msg << "\n";
				app.execute_message(*client, message);
			}
			msg.clear();
			continue ;
		}

		while (not msg.empty() && crlf_indx != msg.npos)
		{
			std::string msg_substr = msg.substr(0, crlf_indx);
			std::cout << "Recv msg from uuid:" << client->uuid << " ->" << msg_substr << "\n";

			if (-1 == app.parse_message(*client, msg_substr, message))
				std::cerr << "Cannot parse message from uuid:" << client->uuid << " ->" << msg_substr << "\n";
			else
			{
				std::cout << "Exec msg from uuid:" << client->uuid << " ->" << msg_substr << "\n";
				app.execute_message(*client, message);
			}
			msg.erase(0, crlf_indx + 2);
			crlf_indx = msg.find(CRLF);
		}
	}
	while (bytes_read && crlf_indx != msg.npos);
}

void close_conn(App &app, int fd)
{
	Client *client = app.find_client_by_fd(fd);
	handle_msg(app, client);
	close(fd);
	std::cout << "Peer with uuid:" << client->uuid << " closed the connection.\n";
	app.remove_client(client->uuid);
}
