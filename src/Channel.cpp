#include "Channel.hpp"

#include <algorithm>

Channel::Channel(std::string const &name) : name(name) {}

void Channel::set_topic(std::string const &topic)
{
    this->topic = topic;
}

std::string Channel::get_topic(void) const
{
    return topic;
}

std::string Channel::get_name(void) const
{
    return name;
}

void Channel::add_client(std::string const &nick)
{
    if (std::find(clients.begin(), clients.end(), nick) == clients.end())
        clients.push_back(nick);
}

std::vector<std::string> const &Channel::get_clients(void) const
{
    return clients;
}

/*
Channels names are strings (beginning with a '&' or '#' character) of
   length up to 200 characters.  Apart from the the requirement that the
   first character being either '&' or '#'; the only restriction on a
   channel name is that it may not contain any spaces (' '), a control G
   (^G or ASCII 7), or a comma (',' which is used as a list item
   separator by the protocol).
*/
bool Channel::is_valid_channel_name(std::string const &name) const
{
    return true;
}
