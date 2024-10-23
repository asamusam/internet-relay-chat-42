#ifndef APP_HPP
#define APP_HPP

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"
#include "IRCReply.hpp"

#include <map>
#include <string>
#include <vector>

class App
{
    public:
        struct Command
        {
            std::string name;
            void (App::*cmd_func)(Client &user, std::vector<std::string> const &params);
        };
        static const int nick_max_len = 9;
        static const int user_max_len = 12;
        static const int client_channel_limit = 10;

    private:
        std::string server_name;
        std::string server_password;
        std::vector<Command> commands;
        std::map<int, Client *> clients;
        std::map<std::string, Channel *> channels;

    public:
        App(std::string const &name, std::string const &password);
        ~App();

        void add_client(Client *new_client);
        void add_channel(Channel *new_channel);

        int parse_message(Client &user, std::string const &msg_string, Message &msg) const;
        void execute_message(Client &user, Message const &msg);
        

        void pass(Client &user, std::vector<std::string> const &params);
        void nick(Client &user, std::vector<std::string> const &params);
        void user(Client &user, std::vector<std::string> const &params);
        void join(Client &user, std::vector<std::string> const &params);
        void privmsg(Client &user, std::vector<std::string> const &params);
        void kick(Client &user, std::vector<std::string> const &params);
        void invite(Client &user, std::vector<std::string> const &params);
        void topic(Client &user, std::vector<std::string> const &params);
        void mode(Client &user, std::vector<std::string> const &params);

        bool is_valid_nick(std::string const &nickname) const;
        bool is_valid_channel_name(std::string const &channel_name) const;
        Client *find_client_by_nick(std::string const &nick) const;
        Client *find_client_by_fd(int fd) const;

        void send_numeric_reply(Client const &user, IRCReplyCodeEnum code, std::map<std::string, std::string> const &info) const;
        void send_msg_to_targets(Client const &user, std::string const &msg, std::vector<std::string> const &targets) const;
        int send_message(Client const &user, std::string const &message) const;

        void free_clients(void);
        void free_channels(void);
};

#endif // APP_HPP