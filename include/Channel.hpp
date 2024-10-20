#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>

// struct Channel
// {
//     std::string name;
//     std::string topic;
//     std::vector<int> client_ids;
// };

class Channel
{
    private:
        std::string topic;
        std::string key;
        std::vector<std::string> clients;
        std::vector<std::string> operators;
        std::vector<std::string> invited;
        std::map<char, bool> mode;
        unsigned int user_limit;

    public:
        std::string name;
    
    public:
        Channel(std::string const &name);

        std::string get_topic(void) const;

        void set_topic(std::string const &topic);
        void set_user_limit(int limit);

        void add_client(std::string const &nick);
        std::vector<std::string> const &get_clients(void) const;

        bool is_invite_only(void) const;
        bool is_full(void) const;
        bool is_key_protected(void) const;

        bool has_user(std::string const &nick) const;
        bool is_invited_user(std::string const &nick) const;
        bool is_matching_key(std::string const &key) const;
        bool is_valid_channel_name(std::string const &name) const;

        void add_to_invited(std::string const &nick);
        void remove_from_invited(std::string const &nick);
};

#endif // CHANNEL_HPP