#include "Channel.hpp"
#include "IRCReply.hpp"

#include <algorithm>
#include <limits>

Channel::Channel(std::string const &name)
{
    this->name = name;
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

int Channel::get_client_count(void) const
{
    return clients.size();
}

void Channel::add_client(std::string const &nick)
{
        clients.push_back(nick);
}

void Channel::remove_client(std::string const &nick)
{
    std::vector<std::string>::iterator it;

    it = std::find(clients.begin(), clients.end(), nick);
    clients.erase(it);
}

std::vector<std::string> const &Channel::get_client_nicks(void) const
{
    return clients;
}

std::string Channel::get_client_nicks_str(void) const
{
    std::string res;

    for (std::vector<std::string>::const_iterator i = clients.begin(); i < clients.end(); i++)
    {
        if (i < clients.end() - 1)
            res += *i + ' ';
        else
            res += *i;
    }
    return res;
}

bool Channel::is_invite_only(void) const
{
    if (mode.find('i') == std::string::npos)
        return false;
    return true;
}

bool Channel::is_invited(std::string const &nick) const
{
    if (std::find(invitations.begin(), invitations.end(), nick) == invitations.end())
        return false;
    return true;
}

void Channel::add_invitation(std::string const &nick)
{
    if (std::find(invitations.begin(), invitations.end(), nick) == invitations.end())
        invitations.push_back(nick);
}

void Channel::remove_invitation(std::string const &nick)
{
    std::vector<std::string>::iterator it;

    it = std::find(invitations.begin(), invitations.end(), nick);
    if (it == invitations.end())
        return ;
    invitations.erase(it);
}

void Channel::add_operator(std::string const &nick)
{
    if (std::find(operators.begin(), operators.end(), nick) == operators.end())
        operators.push_back(nick);
}

void Channel::remove_operator(std::string const &nick)
{
    std::vector<std::string>::iterator it;

    it = std::find(operators.begin(), operators.end(), nick);
    if (it == operators.end())
        return ;
    operators.erase(it);
}

bool Channel::is_full(void) const
{
    if (mode.find('l') == std::string::npos)
        return false;
        return clients.size() == user_limit;
}

void Channel::set_user_limit(int limit)
{
    user_limit = limit;
}

bool Channel::is_key_protected(void) const
{
    if (mode.find('k') == std::string::npos)
        return false;
    return true;
}

// TODO: hashing?
bool Channel::is_matching_key(std::string const &key) const
{
    if (this->key == key)
        return true;
    return false;
}

bool Channel::is_on_channel(std::string const &nick) const
{
    if (std::find(clients.begin(), clients.end(), nick) == clients.end())
        return false;
    return true;
}

bool Channel::is_channel_operator(std::string const &nick) const
{
    if (std::find(operators.begin(), operators.end(), nick) == operators.end())
        return false;
    return true;
}

void Channel::add_mode(char new_mode)
{
    if (mode.find(new_mode) == std::string::npos)
        mode.insert(mode.end(), new_mode);
}

void Channel::remove_mode(char new_mode)
{
    size_t index;

    index = mode.find(new_mode);
    if (index == std::string::npos)
        return;
    mode.erase(index);
}