#include "SystemCallErrorMessage.hpp"

std::pair<scem_function, std::string> sf_data[] = {
	std::make_pair(SCEM_ACCEPT4,      "accept4()"),
	std::make_pair(SCEM_BIND,         "bind()"),
	std::make_pair(SCEM_EPOLL_CREATE, "epoll_create()"),
	std::make_pair(SCEM_EPOLL_CTL,    "epoll_ctl()"),
	std::make_pair(SCEM_EPOLL_WAIT,   "epoll_wait()"),
	std::make_pair(SCEM_LISTEN,       "listen()"),
	std::make_pair(SCEM_RECV,         "recv()"),
	std::make_pair(SCEM_SOCKET,       "socket()")
};

std::map<scem_function, std::string> SystemCallErrorMessage::error_function(sf_data, sf_data + sizeof sf_data / sizeof sf_data[0]);

const std::string& SystemCallErrorMessage::get_func_name(scem_function sf)
{
	return (error_function[sf]);
}
