#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

struct Client
{
    int uuid;
    int fd;
    bool is_registered;
    bool has_valid_pwd;
    std::string username;
    std::string nickname;
	std::string msg_buff;
    int num_channels;
};

#endif // CLIENT_HPP

