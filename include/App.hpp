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
            int (App::*check_cmd)(Client const &user, std::string const &cmd, std::string const &params) const;
            int (App::*execute_cmd)(Message const &msg);
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

        int parse_message(std::string const &msg, Client const &user, Message &res_msg) const;
        bool is_valid_prefix(Client const &user, std::string const &prefix) const;

        int check_command(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_pass(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_nick(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_user(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_join(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_privmsg(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_kick(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_invite(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_topic(Client const &user, std::string const &cmd, std::string const &params) const;
        int check_mode(Client const &user, std::string const &cmd, std::string const &params) const;

        int get_client_id_by_name(std::string const &name) const;
};

#endif // APP_HPP