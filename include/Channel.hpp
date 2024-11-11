#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "App.hpp"

#include <string>
#include <vector>
#include <map>
#include <stack>

enum chan_mode_enum {
    INVITE_ONLY = 1 << 0,
    TOPIC_LOCK  = 1 << 1,
    CHANNEL_KEY = 1 << 2,
    USER_LIMIT  = 1 << 3,
    CHAN_OP     = 1 << 4
};

class Channel
{
    public:

        typedef struct chan_mode_map_s
        {
            chan_mode_enum    mode;
            char              mode_char;
            char              mode_type;
        } chan_mode_map_t;

        typedef struct chan_mode_set_s
        {
            unsigned short mode;
            unsigned int user_limit;
            std::map<chan_mode_enum, std::map<std::string, std::stack<char> > > type_b_params;
            std::map<chan_mode_enum, std::string> type_c_params;
        } chan_mode_set_t;

    private:
        App &app;
        std::vector<Client *> clients;
        std::vector<Client *> invites;
        unsigned short mode;
        unsigned int user_limit;
        std::map<chan_mode_enum, std::vector<std::string> > type_b_params;
        std::map<chan_mode_enum, std::string> type_c_params;
        std::string topic;

    public:
        std::string name;
        static chan_mode_map_t supported_modes[5];
    
    public:
        Channel(App &app, std::string const &nick, std::string const &name);

        std::string const &get_topic(void) const;
        int get_user_limit(void) const;
        int get_client_count(void) const;
        std::string get_client_nicks_str(void) const;

        void set_topic(std::string const &topic);
        void set_user_limit(int limit);

        void add_client(Client *client);
        void remove_client(Client *client);

        bool is_full(void) const;
        bool is_in_mode(chan_mode_enum mode) const;
        bool is_matching_key(std::string const &key) const;
        bool is_on_channel(Client const *client) const;
        bool is_invited(Client const *client) const;
        bool is_channel_operator(std::string const &nick) const;
        static bool is_valid_channel_name(std::string const &channel_name);

        void add_invite(Client *client);
        void remove_invite(Client *client);

        void get_mode_with_params(std::string const &nick, std::map<std::string, std::string> &info) const;
        chan_mode_set_t parse_mode(Client const &user, std::string const &mode_str, std::vector<std::string> const &params) const;
        std::string change_mode(chan_mode_set_t const &channel_mode_set);
        static bool mode_str_has_enough_params(std::string const &mode_str, size_t param_count);
        static bool mode_requires_param(char mode, char sign);
        static void parse_type_b_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param);
        static void parse_type_c_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param);
        static void parse_type_d_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign);

        std::string get_type_c_param(chan_mode_enum mode) const;
        void set_type_c_param(chan_mode_enum mode, std::string const &value);
        void add_type_b_param(chan_mode_enum mode, std::string const &value);
        void remove_type_b_param(chan_mode_enum mode, std::string const &value);
        bool is_type_b_param(chan_mode_enum mode, std::string const &value) const;

        void privmsg(std::string const &source, std::string const &msg) const;
        void notify(std::string const &source, std::string const &cmd, std::string const &param) const;
};

#endif // CHANNEL_HPP