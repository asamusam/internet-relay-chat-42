#include "Channel.hpp"
#include "Errors.hpp"

#include <algorithm>
#include <limits>

Channel::Channel(std::string const &name)
{
    if (!is_valid_channel_name(name))
        throw ERR_BADCHANMASK;
    this->name = name;
    mode['i'] = false;
    mode['t'] = false;
    mode['k'] = false;
    mode['l'] = false;
    user_limit = std::numeric_limits<unsigned int>::max();
}

void Channel::set_topic(std::string const &topic)
{
    this->topic = topic;
}

std::string Channel::get_topic(void) const
{
    return topic;
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
	(void) name;
    return true;
}

bool Channel::is_invite_only(void) const
{
    return mode.find('i')->second;
}

bool Channel::is_invited_user(std::string const &nick) const
{
    if (std::find(invited.begin(), invited.end(), nick) == invited.end())
        return false;
    return true;
}

void Channel::add_to_invited(std::string const &nick)
{
    if (std::find(invited.begin(), invited.end(), nick) == invited.end())
        invited.push_back(nick);
}

void Channel::remove_from_invited(std::string const &nick)
{
    std::vector<std::string>::iterator it;

    it = std::find(invited.begin(), invited.end(), nick);
    if (it == invited.end())
        return ;
    invited.erase(it);
}

bool Channel::is_full(void) const
{
    if (mode.find('l')->second)
        return clients.size() == user_limit;
    return false;
}

void Channel::set_user_limit(int limit)
{
    user_limit = limit;
}

bool Channel::is_key_protected(void) const
{
    return mode.find('k')->second;
}

// TODO: hashing?
bool Channel::is_matching_key(std::string const &key) const
{
    if (this->key == key)
        return true;
    return false;
}

bool Channel::has_user(std::string const &nick) const
{
    if (std::find(clients.begin(), clients.end(), nick) == clients.end())
        return false;
    return true;
}

