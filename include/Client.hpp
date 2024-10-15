#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

struct Client
{
    int uuid;
    int fd;
    bool registered;
    bool password;
    std::string username;
    std::string nickname;
};

#endif // CLIENT_HPP