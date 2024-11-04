#ifndef CHANNEL_HPP
#define CHANNEL_HPP

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

typedef struct chan_mode_map_s
{
    chan_mode_enum    mode;
    char              mode_char;
    char              mode_type;
} chan_mode_map_t;

class Channel
{
    private:
        std::vector<std::string> clients;
        std::vector<std::string> invitations;
        unsigned short mode;
        unsigned int user_limit;
        std::map<chan_mode_enum, std::vector<std::string> > type_b_params;
        std::map<chan_mode_enum, std::string> type_c_params;
        std::string topic;

    public:
        std::string name;
        static chan_mode_map_t supported_modes[5];
    
    public:
        Channel(std::string const &name);

        std::string const &get_topic(void) const;
        void set_topic(std::string const &topic);

        int get_client_count(void) const;

        int get_user_limit(void) const;
        void set_user_limit(int limit);

        void add_client(std::string const &nick);
        void remove_client(std::string const &nick);

        std::vector<std::string> const &get_client_nicks(void) const;
        std::string get_client_nicks_str(void) const;

        bool is_full(void) const;
        bool is_in_mode(chan_mode_enum mode) const;
        bool is_matching_key(std::string const &key) const;
        bool is_on_channel(std::string const &nick) const;
        bool is_invited(std::string const &nick) const;
        bool is_channel_operator(std::string const &nick) const;

        void add_invitation(std::string const &nick);
        void remove_invitation(std::string const &nick);

        unsigned short get_mode(void) const;
        void set_mode(unsigned short mode);
        void get_mode_with_params(std::string const &nick, std::map<std::string, std::string> &info) const;

        std::string get_type_c_param(chan_mode_enum mode) const;
        void set_type_c_param(chan_mode_enum mode, std::string const &value);

        void add_type_b_param(chan_mode_enum mode, std::string const &value);
        void remove_type_b_param(chan_mode_enum mode, std::string const &value);
        bool is_type_b_param(chan_mode_enum mode, std::string const &value) const;

        std::string get_key(void) const;

};

#endif // CHANNEL_HPP