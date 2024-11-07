#ifndef APP_HPP
#define APP_HPP

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"
#include "IRCReply.hpp"

#include <map>
#include <string>
#include <vector>

#define CRLF "\r\n"

typedef struct chan_mode_set_s
{
    unsigned short mode;
    unsigned int user_limit;
    std::map<chan_mode_enum, std::map<std::string, std::stack<char> > > type_b_params;
    std::map<chan_mode_enum, std::string> type_c_params;
} chan_mode_set_t;

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
        std::map<uint32, Client *> clients;
        std::map<std::string, Channel *> channels;

    public:
        App(std::string const &name, std::string const &password);
        ~App();

        void add_client(Client *new_client);
        void remove_client(int uuid);

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
        void ping(Client &user, std::vector<std::string> const &params);

        bool is_valid_nick(std::string const &nickname) const;
        bool is_valid_channel_name(std::string const &channel_name) const;

        Client *find_client_by_nick(std::string const &nick) const;
        Client *find_client_by_fd(int fd) const;
		uint32 generate_uuid(void) const;

        Channel *find_channel_by_name(std::string const &channel_name) const;

        void send_numeric_reply(Client const &client, IRCReplyCodeEnum code, std::map<std::string, std::string> const &info) const;
        void fill_placeholders(std::string &str, std::map<std::string, std::string> const &info) const;

        static std::string create_message(std::string const &prefix, std::string const &cmd, std::string const &msg);
        static void send_message(Client const &user, std::string const &msg);
        void notify_channel(Channel *channel, std::string const &source, std::string const &cmd, std::string const &param) const;

        void free_clients(void);
        void free_channels(void);

        static bool mode_str_has_enough_params(std::string const &mode_str, size_t param_count);
        static bool mode_requires_param(char mode, char sign);
        void parse_type_b_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param) const;
        static void parse_type_c_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param);
        static void parse_type_d_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign);
        chan_mode_set_t parse_mode_string(Client const &user, Channel const *channel, std::string const &mode_str, std::vector<std::string> const &params) const;
        std::string change_channel_mode(Channel *channel, chan_mode_set_t const &channel_mode_set);
        
        void send_privmsg(Client const &user, std::string const &msg, std::vector<std::string> const &targets) const;
        void privmsg_channel(Channel *channel, std::string const &source, std::string const &msg) const;
        void privmsg_client(Client *client, std::string const &source, std::string const &msg) const;
};

#endif // APP_HPP
