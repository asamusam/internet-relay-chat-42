#ifndef APP_HPP
#define APP_HPP

#include "Message.hpp"
#include "IRCReply.hpp"

#include <map>
#include <string>
#include <vector>

#define CRLF "\r\n"

class Channel;
class Client;
typedef unsigned long uint32;


class App
{
    public:
        struct Command
        {
            std::string name;
            void (Client::*cmd_func)(std::vector<std::string> const &params);
        };
        static const int nick_max_len = 9;
        static const int user_max_len = 12;
        static const int client_channel_limit = 10;

    private:
        std::string server_password;
        std::vector<Command> commands;
        std::map<uint32, Client *> clients;
        std::map<std::string, Channel *> channels;

    public:
        std::string server_name;
        std::string server_version;
        std::string created_at;
        std::string network_name;

    public:
        App(std::string const &name, std::string const &password);
        ~App();

        void add_client(Client *new_client);
        void remove_client(uint32 uuid);

        Channel *create_channel(std::string const &nick, std::string const &channel_name);
        void add_channel(Channel *channel);
        void remove_channel(std::string const &channel_name);

        int parse_message(Client &user, std::string const &msg_string, Message &msg) const;
        void execute_message(Client &user, Message const &msg);

		Client *get_client(uint32 uuid) const;
        Client *find_client_by_nick(std::string const &nick) const;
        Client *find_client_by_fd(int fd) const;

        Channel *find_channel_by_name(std::string const &channel_name) const;

        void free_clients(void);
        void free_channels(void);
        
        static std::string create_message(std::string const &prefix, std::string const &cmd, std::string const &msg);

        bool is_correct_pwd(std::string const &password) const;

};

#endif // APP_HPP
