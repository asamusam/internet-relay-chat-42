#include <cerrno>
#include <cstring>
#include <iostream>
#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include "App.hpp"
#include "InternalError.hpp"
#include "SystemCallErrorMessage.hpp"
#include "connection.hpp"

int g_listen_sock_fd = -1;
App *g_app = NULL;

void conn_loop(App &app, int listen_sock_fd)
{
	g_app = &app;
	int nfds = 0;
	#ifdef __APPLE__
	struct kevent events[ConnConst::max_events];
	#else
	struct epoll_event events[ConnConst::max_events];
	#endif
	(void) std::memset(events, 0, sizeof(events));

	int epoll_fd = epoll_init(listen_sock_fd);

	for (;;)
	{
		#ifdef __APPLE__
		nfds = kevent(epoll_fd, NULL, 0, events, ConnConst::max_events, NULL);
		if (-1 == nfds)
			throw (SCEM_KEVENT);
		#else
		nfds = epoll_wait(epoll_fd, events, ConnConst::max_events, ConnConst::time_out_ms);
		if (-1 == nfds)
			throw (SCEM_EPOLL_WAIT);
		#endif

		for (int i = 0; i < nfds; ++i)
		{
			#ifdef __APPLE__
			int fd = events[i].ident;
			int filter = events[i].filter;
			bool is_hup = events[i].flags & EV_EOF;
			#else
			int fd = events[i].data.fd;
			bool is_hup = events[i].events & (EPOLLHUP | EPOLLRDHUP);
			#endif

			try
			{
				if (fd == listen_sock_fd)
					accept_in_conns(app, epoll_fd, listen_sock_fd);
				else if (is_hup)
					close_conn_by_fd(app, fd);
				#ifdef __APPLE__
				else if (filter == EVFILT_READ)
					handle_msg(app, app.find_client_by_fd(fd));
				#else
				else if (events[i].events & EPOLLIN)
					handle_msg(app, app.find_client_by_fd(fd));
				#endif
			}
			catch (scem_function sf)
			{
				std::cerr << "System error while trying to handle connection: "
					<< SystemCallErrorMessage::get_func_name(sf) << "\n";
			}
			catch (std::out_of_range &e)
			{
				std::cerr << "Error while manupulating strings" << e.what() << "\n";
			}
		}
	}

	close(listen_sock_fd);
}

int main(int argc, char **argv)
{
	try
	{
		setup_signal_handlers();
		if (argc != 3)
			throw (IEC_BADARGC);

		std::string password(argv[2]);
		if (password.size() < 4 || password.size() > 32)
			throw (IEC_BADPASS);

		int listen_sock_fd = listen_sock_init(parse_port(argv[1]));
		g_listen_sock_fd = listen_sock_fd;

		App app("127.0.0.1", password);

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
