#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "App.hpp"

#include <string>


class Client
{
    private:
        App &app;
        uint32 uuid;
        int fd;
        bool is_registered;
        bool has_valid_pwd;
        std::string username;
        std::string nickname;
        std::string full_nickname;
        int num_channels;
        
    public:
        // quick fix, needs to be changed in the future
        std::string msg_buff;

    public:
        Client(App &app, int fd);
        ~Client();

        uint32 generate_uuid(void) const;
        void register_client(void);

        std::string get_nickname(void) const;
        std::string get_full_nickname(void) const;
        int get_fd(void) const;
        uint32 get_uuid(void) const;

        void send_message(std::string const &msg) const;
        void send_numeric_reply(IRCReplyCodeEnum code, std::map<std::string, std::string> const &info) const;

        static void fill_placeholders(std::string &str, std::map<std::string, std::string> const &info);
        static int split_targets(std::string const &target_str, std::vector<std::string> &targets);

        void privmsg_client(std::string const &source, std::string const &msg) const;
        void privmsg_targets(std::string const &msg, std::vector<std::string> const &targets) const;

        void pass(std::vector<std::string> const &params);
        void nick(std::vector<std::string> const &params);
        void user(std::vector<std::string> const &params);
        void join(std::vector<std::string> const &params);
        void privmsg(std::vector<std::string> const &params);
        void kick(std::vector<std::string> const &params);
        void invite(std::vector<std::string> const &params);
        void topic(std::vector<std::string> const &params);
        void mode(std::vector<std::string> const &params);
        void ping(std::vector<std::string> const &params);

        bool is_valid_nick(std::string const &nickname) const;
        bool is_registered_client(void) const;
};

#endif // CLIENT_HPP
