#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

// struct Channel
// {
//     std::string name;
//     std::string topic;
//     std::vector<int> client_ids;
// };

class Channel
{
    private:
        std::string name;
        std::string topic;
        std::vector<std::string> clients;
    public:
        Channel(std::string const &name);

        std::string get_name(void) const;
        std::string get_topic(void) const;

        void set_topic(std::string const &topic);

        void add_client(std::string const &nick);
        std::vector<std::string> const &get_clients(void) const;

        bool is_valid_channel_name(std::string const &name) const;
};

#endif // CHANNEL_HPP