#include "Channel.hpp"
#include "IRCReply.hpp"

#include <algorithm>
#include <limits>

chan_mode_map_t Channel::supported_modes[] = {
    {INVITE_ONLY, 'i', 'd'},
    {TOPIC_LOCK, 't', 'd'},
    {CHANNEL_KEY, 'k', 'c'},
    {USER_LIMIT, 'l', 'c'},
    {CHAN_OP, 'o', 'b'}
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

// CHECK CHANNEL MODE //

bool Channel::is_in_mode(chan_mode_enum mode) const
{
    return this->mode & mode;
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

bool Channel::is_matching_key(std::string const &key) const
{
    return this->get_key() == key;
}

bool Channel::is_on_channel(std::string const &nick) const
{
    if (std::find(clients.begin(), clients.end(), nick) == clients.end())
        return false;
    return true;
}

bool Channel::is_channel_operator(std::string const &nick) const
{
    return is_type_b_param(CHAN_OP, nick);
}

unsigned short Channel::get_mode(void) const
{
    return mode;
}

void Channel::set_mode(unsigned short mode)
{
    this->mode = mode;
}

std::string Channel::get_type_c_param(chan_mode_enum mode) const
{
    if (type_c_params.count(mode) == 0)
        return "";
    return type_c_params.at(mode);
}

void Channel::set_type_c_param(chan_mode_enum mode, std::string const &value)
{
    type_c_params[mode] = value;
    if (mode == USER_LIMIT)
        user_limit = std::atoi(value.c_str());
}

bool Channel::is_type_b_param(chan_mode_enum mode, std::string const &value) const
{
    if (type_b_params.count(mode) == 0)
        return false;
    if (std::find(type_b_params.at(mode).begin(), type_b_params.at(mode).end(), value) == type_b_params.at(mode).end())
        return false;
    return true;
}

void Channel::add_type_b_param(chan_mode_enum mode, std::string const &value)
{
    type_b_params[mode].push_back(value);
}

void Channel::remove_type_b_param(chan_mode_enum mode, std::string const &value)
{
    std::vector<std::string>::iterator it;

    it = std::find(type_b_params[mode].begin(), type_b_params[mode].end(), value);
    if (it == type_b_params[mode].end())
        return ;
    type_b_params[mode].erase(it);
}

void Channel::get_mode_with_params(std::string const &nick, std::map<std::string, std::string> &info) const
{
    std::string mode_string("+");
    std::string params;
    size_t arr_size;

    info["mode"] = "+";
    info["mode params"];
    arr_size = sizeof(supported_modes) / sizeof(chan_mode_map_t);
    for (size_t i = 0; i < arr_size; i++)
    {
        if (this->mode & supported_modes[i].mode)
        {
            info["mode"] += supported_modes[i].mode_char;
            if (supported_modes[i].mode_type == 'c')
            {
                if (supported_modes[i].mode == CHANNEL_KEY && !this->is_channel_operator(nick))
                    continue;
                info["mode params"] += this->get_type_c_param(supported_modes[i].mode) + ' ';
            }
        }
    }
}

std::string Channel::get_key(void) const
{
    return get_type_c_param(CHANNEL_KEY);
}

