#ifndef SYSTEM_CALL_ERROR_MESSAGE_HPP
#define SYSTEM_CALL_ERROR_MESSAGE_HPP

#include <map>
#include <string>

enum scem_function
{
	SCEM_NONE = 0,

	SCEM_ACCEPT4,
	SCEM_BIND,
	SCEM_EPOLL_CREATE,
	SCEM_EPOLL_CTL,
	SCEM_EPOLL_WAIT,
	SCEM_LISTEN,
	SCEM_RECV,
	SCEM_SOCKET
};

class SystemCallErrorMessage
{
	private:
		static std::map<scem_function, std::string> error_function;
	public:
		static const std::string& get_func_name(scem_function);
};

#endif /* SYSTEM_CALL_ERROR_MESSAGE_HPP */
