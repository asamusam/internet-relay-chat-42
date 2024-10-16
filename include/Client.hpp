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
};

#endif // CLIENT_HPP