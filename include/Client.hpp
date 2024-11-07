#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

typedef unsigned long uint32;

struct Client
{
    uint32 uuid;
    int fd;
    bool is_registered;
    bool has_valid_pwd;
    std::string username;
    std::string nickname;
    std::string full_nickname;
	std::string msg_buff;
    int num_channels;
};

#endif // CLIENT_HPP
