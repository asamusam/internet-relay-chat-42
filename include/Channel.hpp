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
        std::vector<std::string> operators;
        std::vector<std::string> invitations;
        unsigned short mode;
        unsigned int user_limit;
        std::string topic;
        std::string key;

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

        std::vector<std::string> const &get_operators(void) const;

        bool is_invite_only(void) const;
        bool is_full(void) const;
        bool is_in_user_limit_mode(void) const;
        bool is_key_protected(void) const;
        bool is_in_topic_protected_mode(void) const;

        bool is_on_channel(std::string const &nick) const;
        bool is_invited(std::string const &nick) const;
        bool is_matching_key(std::string const &key) const;
        bool is_channel_operator(std::string const &nick) const;

        void add_invitation(std::string const &nick);
        void remove_invitation(std::string const &nick);

        void add_operator(std::string const &nick);
        void remove_operator(std::string const &nick);

        unsigned short get_mode(void) const;
        void set_mode(unsigned short mode);
        std::string get_mode_string(std::string const &nick) const;

        void set_key(std::string const &key);
        std::string const &get_key(void) const;

};

#endif // CHANNEL_HPP