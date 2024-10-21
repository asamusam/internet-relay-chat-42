#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/epoll.h>
#include "InternalError.hpp"
#include "SystemCallErrorMessage.hpp"
#include "connection.hpp"
#include "Client.hpp"
#include "App.hpp"

void conn_loop(App &app, int listen_sock_fd)
{
	int nfds = 0;
	struct epoll_event events[ConnConst::max_events];
	(void) std::memset(events, 0, sizeof(events));

	int epoll_fd = epoll_init(listen_sock_fd);

	for (;;)
	{
		nfds = epoll_wait(epoll_fd, events, ConnConst::max_events, ConnConst::time_out_ms);
		if (-1 == nfds)
			throw (SCEM_EPOLL_WAIT);

		for (int i = 0; i < nfds; ++i)
		{
			int fd = events[i].data.fd;

			if (fd == listen_sock_fd)
				accept_in_conns(epoll_fd, listen_sock_fd);
			else
				handle_msg(app.find_client_by_fd(fd));
		}
	}
}

int main(int argc, char **argv)
{
	try
	{
		if (argc != 3)
			throw (IEC_BADARGC);

		std::string password(argv[2]);
		if (password.size() < 8 || password.size() > 32)
			throw (IEC_BADPASS);

		int listen_sock_fd = listen_sock_init(parse_port(argv[1]));

		App app("ft_irc", password);

		conn_loop(app, listen_sock_fd);
	}
	catch (internal_error_code iec)
	{
		std::cerr << "Error: " << InternalError::get_error_message(iec) << "\n";
		return (1);
	}
	catch (scem_function sf)
	{
		std::cerr << "Error while executing a system call: " << SystemCallErrorMessage::get_func_name(sf)
			<< ": " << std::strerror(errno) << "\n";
		return (2);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << "\n";
	}

    return (0);
}
