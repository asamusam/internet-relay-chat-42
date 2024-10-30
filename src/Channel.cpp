#include "Channel.hpp"
#include "IRCReply.hpp"

#include <algorithm>
#include <limits>

channel_mode_map_t Channel::supported_modes[] = {
    {INVITE_ONLY, 'i'},
    {TOPIC_LOCK, 't'},
    {CHANNEL_KEY, 'k'},
    {USER_LIMIT, 'l'}
};

Channel::Channel(std::string const &name)
{
    this->name = name;
    this->mode = 0;
    user_limit = std::numeric_limits<unsigned int>::max();
}

void Channel::set_topic(std::string const &topic)
{
    this->topic = topic;
}

std::string const &Channel::get_topic(void) const
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

std::vector<std::string> const &Channel::get_operators(void) const
{
    return operators;
}

// CHECK CHANNEL MODE //

bool Channel::is_invite_only(void) const
{
    return mode & INVITE_ONLY;
}

bool Channel::is_in_topic_protected_mode(void) const
{
    return mode & TOPIC_LOCK;
}

bool Channel::is_in_user_limit_mode(void) const
{
    return mode & USER_LIMIT;
}

bool Channel::is_key_protected(void) const
{
    return mode & CHANNEL_KEY;
}

// ---------------------- //





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
    if (mode & USER_LIMIT)
        return clients.size() == user_limit;
    return false;
}

int Channel::get_user_limit(void) const
{
    return user_limit;
}

void Channel::set_user_limit(int limit)
{
    user_limit = limit;
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

unsigned short Channel::get_mode(void) const
{
    return mode;
}

void Channel::set_mode(unsigned short mode)
{
    this->mode = mode;
}

void Channel::set_key(std::string const &key)
{
    this->key = key;
}

std::string Channel::get_mode_string(std::string const &nick) const
{
    std::string mode_string("+");
    std::string params;
    size_t arr_size;

    arr_size = sizeof(supported_modes) / sizeof(channel_mode_map_t);
    for (size_t i = 0; i < arr_size; i++)
    {
        if (this->mode & supported_modes[i].mode)
        {
            mode_string += supported_modes[i].mode_char;
            switch (supported_modes[i].mode)
            {
            case CHANNEL_KEY:
                if (this->is_channel_operator(nick))
                    params += ' ' + this->key;
                break;
            case USER_LIMIT:
                params += ' ' + this->user_limit;
            
            default:
                break;
            }
        }
    }
    return mode_string + params;
}


std::string const &Channel::get_key(void) const
{
    return key;
}

