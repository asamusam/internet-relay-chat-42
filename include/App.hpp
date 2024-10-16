#ifndef APP_HPP
#define APP_HPP

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"
#include "Errors.hpp"

#include <map>
#include <string>
#include <vector>

class App
{
    public:
        struct Command
        {
            std::string name;
            int (App::*cmd_func)(Client &user, std::vector<std::string> const &params);
        };

    private:
        std::string server_name;
        std::string server_password;
        std::vector<Command> commands;
        std::map<int, std::string> error_messages;
        std::map<int, Client *> clients;
        std::vector<Channel *> channels;

    public:
        App(std::string const &name, std::string const &password);

        void add_client(Client *new_client);

        int parse_message(Client &user, std::string const &msg_string, std::string &reply);
        int run_cmd(Client &user, Message const &msg);
        int pass(Client &user, std::vector<std::string> const &params);
        int nick(Client &user, std::vector<std::string> const &params);
        int user(Client &user, std::vector<std::string> const &params);
        int join(Client &user, std::vector<std::string> const &params);
        int privmsg(Client &user, std::vector<std::string> const &params);
        int kick(Client &user, std::vector<std::string> const &params);
        int invite(Client &user, std::vector<std::string> const &params);
        int topic(Client &user, std::vector<std::string> const &params);
        int mode(Client &user, std::vector<std::string> const &params);

        bool nick_is_valid(std::string const &nick) const;
        Client *find_client_by_nick(std::string const &nick) const;
};

#endif // APP_HPP