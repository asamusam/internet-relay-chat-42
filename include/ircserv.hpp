#ifndef IRCSERV_HPP
#define IRCSERV_HPP

#include <string>
#include <vector>

typedef struct sMessage
{
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
} message;

typedef struct sClient
{
    int uuid;
    int fd;
    bool registered;
    bool password;
    std::string username;
    std::string nickname;
} client;

typedef struct sChannel
{
    std::string name;
    std::string topic;
    std::vector<int> clientIds;
} channel;

typedef struct sApp
{
    std::string serverName;
    std::vector<client *> clients;
    std::vector<channel *> channels;
} app;

int parseMessage(app const &data, client const &user, std::string const &msg, message &resMsg);

#endif