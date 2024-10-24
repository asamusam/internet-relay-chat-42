#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>

class Channel
{
    private:
        std::string topic;
        std::string key;
        std::vector<std::string> clients;
        std::vector<std::string> operators;
        std::vector<std::string> invitations;
        std::string modes;
        unsigned int user_limit;

    public:
        std::string name;
    
    public:
        Channel(std::string const &name);

        std::string const &get_topic(void) const;
        void set_topic(std::string const &topic);

        int get_client_count(void) const;
        void set_user_limit(int limit);

        void add_client(std::string const &nick);
        void remove_client(std::string const &nick);

        std::vector<std::string> const &get_client_nicks(void) const;
        std::string get_client_nicks_str(void) const;

        bool is_invite_only(void) const;
        bool is_full(void) const;
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

        void add_mode(char mode);
        void remove_mode(char mode);
        std::string const &get_modes(void) const;

        void set_key(std::string const &key);
        std::string const &get_key(void) const;

};

#endif // CHANNEL_HPP