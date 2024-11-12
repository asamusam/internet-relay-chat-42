#include <cstdlib>
#include <cerrno>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <limits>
#include <netinet/in.h>
#include <sstream>
#ifdef __APPLE__
#include <sys/event.h>
#include <fcntl.h>
#else
#include <sys/epoll.h>
#endif
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

    #ifdef __APPLE__
    sock_fd = socket(sai.sin_family, SOCK_STREAM, 0);
    if (-1 == sock_fd)
        throw (SCEM_SOCKET);

    if (fcntl(sock_fd, F_SETFL, O_NONBLOCK) == -1)
        throw (SCEM_FCNTL);
    #else
	sock_fd = socket(sai.sin_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (-1 == sock_fd)
		throw (SCEM_SOCKET);
    #endif

	if (-1 == bind(sock_fd, (struct sockaddr *) &sai, sizeof(sai)))
		throw (SCEM_BIND);

	if (-1 == listen(sock_fd, ConnConst::max_conns))
		throw (SCEM_LISTEN);

	return (sock_fd);
}

int epoll_init(int listen_sock_fd)
{

    #ifdef __APPLE__
    int kq_fd = -1;
    struct kevent ev;
	(void) std::memset(&ev, 0, sizeof(ev));

    kq_fd = kqueue();
    if (kq_fd == -1)
        throw (SCEM_KQUEUE);
    
    EV_SET(&ev, listen_sock_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kq_fd, &ev, 1, NULL, 0, NULL) == -1)
        throw (SCEM_KEVENT);

    return (kq_fd);
    #else
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
    #endif
}

void accept_in_conns(App &app, int epoll_fd, int listen_sock_fd)
{
    #ifdef __APPLE__
    struct kevent ev;
	(void) std::memset(&ev, 0, sizeof(ev));

    int conn_sock_fd = accept(listen_sock_fd, NULL, NULL);
    if (-1 == conn_sock_fd)
        throw (SCEM_ACCEPT);

    if (fcntl(conn_sock_fd, F_SETFL, O_NONBLOCK) == -1)
        throw (SCEM_FCNTL);

    EV_SET(&ev, conn_sock_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(epoll_fd, &ev, 1, NULL, 0, NULL) == -1)
        throw (SCEM_KEVENT);
    #else
	epoll_event ev;
	(void) std::memset(&ev, 0, sizeof(ev));

	int conn_sock_fd = accept4(listen_sock_fd, NULL, NULL, SOCK_NONBLOCK);	
	if (-1 == conn_sock_fd)
		throw (SCEM_ACCEPT4);

	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	ev.data.fd = conn_sock_fd;
	if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock_fd, &ev))
		throw (SCEM_EPOLL_CTL);
    #endif

	Client *client = new Client(app, conn_sock_fd);
	app.add_client(client);

	std::cout << "ACCEPT'ed new connection and created new client with uuid:" << client->pretty_uuid() << " and fd:"
		<< client->get_fd() << "\n";
}

void handle_msg(App &app, Client *client)
{
	char buff[MAX_MSG_SIZE];
	std::string msg = client->get_msg_buff();
	ssize_t bytes_read;
	size_t crlf_indx;
	Message message;

	do
	{
		std::memset(buff, 0, sizeof buff);
		bytes_read = recv(client->get_fd(), buff, sizeof buff, 0);
		std::cout << "RECV chars from uuid:" << client->pretty_uuid() << " ->" << buff
			<< (std::strchr(buff, '\n') ? "" : "\n") ;
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
			std::cout << "Completed msg from uuid:" << client->pretty_uuid() << " ->" << msg << "\n";
			if (-1 == app.parse_message(*client, msg, message))
				std::cerr << "Cannot parse message from uuid:" << client->pretty_uuid() << " ->" << msg << "\n";
			else
			{
				std::cout << "EXEC msg from uuid:" << client->pretty_uuid() << " ->" << msg << "\n";
				app.execute_message(*client, message);
			}
			msg.clear();
			continue ;
		}

		while (not msg.empty() && crlf_indx != msg.npos)
		{
			std::string msg_substr = msg.substr(0, crlf_indx);
			std::cout << "Completed msg from uuid:" << client->pretty_uuid() << " ->" << msg_substr << "\n";

			if (-1 == app.parse_message(*client, msg_substr, message))
				std::cerr << "Cannot parse message from uuid:" << client->pretty_uuid() << " ->" << msg_substr << "\n";
			else
			{
				std::cout << "EXEC msg from uuid:" << client->pretty_uuid() << " ->" << msg_substr << "\n";
				app.execute_message(*client, message);
			}
			msg.erase(0, crlf_indx + 2);
			crlf_indx = msg.find(CRLF);
		}
	}
	while (crlf_indx != msg.npos);
	client->set_msg_buff(msg);
}

void close_conn_by_fd(App &app, int fd)
{
	Client *client = app.find_client_by_fd(fd);
	handle_msg(app, client);
	close(fd);
	std::cout << "Peer with uuid:" << client->pretty_uuid() << " closed the connection.\n";
	app.remove_client(client->get_uuid());
}

void close_conn_by_uuid(App &app, uint32 uuid)
{
	Client *client = app.get_client(uuid);
	handle_msg(app, client);
	close(client->get_fd());
	std::cout << "Closed connection to peer with uuid:" << client->pretty_uuid() << ".\n";
	app.remove_client(client->get_uuid());
}

static void signal_handler(int sig)
{
	if (sig == SIGQUIT || sig == SIGINT)
	{
		//TODO clean. pass App as global variable.
		std::exit(0);
	}
}

void setup_signal_handlers(void)
{
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));

	sa.sa_handler = &signal_handler;

	if (-1 == sigaction(SIGINT, &sa, NULL))
		throw(SCEM_SIGACT);
	if (-1 == sigaction(SIGQUIT, &sa, NULL))
		throw(SCEM_SIGACT);
}
