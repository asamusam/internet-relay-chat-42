#include "Channel.hpp"
#include "IRCReply.hpp"

#include <algorithm>
#include <limits>

Channel::Channel(std::string const &name)
{
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
    return mode.find('i')->second;
}

bool Channel::is_invited_user(std::string const &nick) const
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

