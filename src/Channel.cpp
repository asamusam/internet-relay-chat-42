#include "Channel.hpp"
#include "Client.hpp"
#include "IRCReply.hpp"

#include <algorithm>
#include <limits>


// ============================
//         STATIC DATA
// ============================

Channel::chan_mode_map_t Channel::supported_modes[] = {
    {INVITE_ONLY, 'i', 'd'},
    {TOPIC_LOCK, 't', 'd'},
    {CHANNEL_KEY, 'k', 'c'},
    {USER_LIMIT, 'l', 'c'},
    {CHAN_OP, 'o', 'b'}
};


// ============================
//         CONSTRUCTOR
// ============================

Channel::Channel(App &app, std::string const &nick, std::string const &name): app(app), name(name)
{
    this->mode = 0;
    this->topic = ":";
    user_limit = std::numeric_limits<unsigned int>::max();
    add_type_b_param(CHAN_OP, nick);
}


// ============================
//          CLIENTS
// ============================

void Channel::add_client(Client *client)
{
    clients.push_back(client);
}

void Channel::remove_client(Client *client)
{
    std::vector<Client *>::iterator it;

    it = std::find(clients.begin(), clients.end(), client);
    clients.erase(it);
    if (clients.size() == 0)
        app.remove_channel(this->name);
}


// ============================
//          INVITES
// ============================

void Channel::add_invite(Client *client)
{
    if (is_invited(client))
        return ;
    invites.push_back(client);
}

void Channel::remove_invite(Client *client)
{
    std::vector<Client *>::iterator it;

    it = std::find(invites.begin(), invites.end(), client);
    if (it == invites.end())
        return ;
    invites.erase(it);
}


// ============================
//          GETTERS
// ============================

int Channel::get_user_limit(void) const
{
    return user_limit;
}

std::string Channel::get_client_nicks_str(void) const
{
    std::string res;

    for (std::vector<Client *>::const_iterator i = clients.begin(); i < clients.end(); i++)
    {
        if (i < clients.end() - 1)
            res += (*i)->get_nickname() + ' ';
        else
            res += (*i)->get_nickname();
    }
    return res;
}

std::string const &Channel::get_topic(void) const
{
    return topic;
}

int Channel::get_client_count(void) const
{
    return clients.size();
}


// ============================
//          SETTERS
// ============================

void Channel::set_user_limit(int limit)
{
    user_limit = limit;
}

void Channel::set_topic(std::string const &topic)
{
    this->topic = topic;
}


// ============================
//          CHECKERS
// ============================

bool Channel::is_matching_key(std::string const &key) const
{
    return get_type_c_param(CHANNEL_KEY) == key;
}

bool Channel::is_on_channel(Client const *client) const
{
    return std::find(clients.begin(), clients.end(), client) != clients.end();
}

bool Channel::is_channel_operator(std::string const &nick) const
{
    return is_type_b_param(CHAN_OP, nick);
}

bool Channel::is_invited(Client const *client) const
{
    return std::find(invites.begin(), invites.end(), client) != invites.end();
}

bool Channel::is_full(void) const
{
    if (is_in_mode(USER_LIMIT))
        return clients.size() == user_limit;
    return false;
}

bool Channel::is_in_mode(chan_mode_enum mode) const
{
    return this->mode & mode;
}

/*
Channels names are strings (beginning with a '&' or '#' character) of
length up to 200 characters.  Apart from the the requirement that the
first character being either '&' or '#'; the only restriction on a
channel name is that it may not contain any spaces (' '), a control G
(^G or ASCII 7), or a comma (',' which is used as a list item
separator by the protocol).
*/
bool Channel::is_valid_channel_name(std::string const &channel_name)
{
    if (channel_name[0] != '&' && channel_name[0] != '#')
        return false;
    for (std::size_t i = 1; i < channel_name.length(); ++i)
    {
        if (channel_name[i] == ' ' || channel_name[i] == '\x07' || channel_name[i] == ',')
            return false;
    }
    return true;
}

// ============================
//            PARAMS
// ============================

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


// ============================
//            MODE
// ============================

Channel::chan_mode_set_t Channel::parse_mode(Client const &user, std::string const &mode_str, std::vector<std::string> const &params) const
{
    std::map<std::string, std::string> info;
    chan_mode_set_t mode_set;
    Client *target;

    mode_set.mode = mode;
    mode_set.type_c_params[CHANNEL_KEY] = get_type_c_param(CHANNEL_KEY);
    mode_set.user_limit = user_limit;
    info["channel"] = name;
    info["client"] = user.get_full_nickname();

    char sign = mode_str[0];
    size_t index = 2;
    size_t supported_modes_size = sizeof(supported_modes) / sizeof(chan_mode_map_t);
    for (std::string::const_iterator ch = mode_str.begin(); ch < mode_str.end(); ch++)
    {
        if (*ch == '+' || *ch == '-')
            sign = *ch;
        else
        {
            bool unknown = true;
            for (size_t i = 0; i < supported_modes_size; i++)
            {
                if (supported_modes[i].mode_char == *ch)
                {
                    switch (supported_modes[i].mode_type)
                    {
                    case 'b':
                        target = app.find_client_by_nick(params[index]);
                        info["nick"] = params[index++];
                        if (!target)
                            user.send_numeric_reply(ERR_NOSUCHNICK, info);
                        else if (!is_on_channel(target))
                            user.send_numeric_reply(ERR_NOTONCHANNEL, info);
                        else
                            parse_type_b_mode(mode_set, supported_modes[i].mode, sign, target->get_nickname());
                        break;
                    
                    case 'c':
                        if (*ch == 'k' && sign == '+' && mode_set.mode & CHANNEL_KEY)
                            user.send_numeric_reply(ERR_KEYSET, info);
                        else
                            parse_type_c_mode(mode_set, supported_modes[i].mode, sign, params[index]);
                        if (sign == '+')
                            ++index;
                        break;
                    
                    case 'd':
                        parse_type_d_mode(mode_set, supported_modes[i].mode, sign);
                        break;
                    
                    default:
                        break;                    
                    }
                    unknown = false;
                    break;
                }
            }
            if (unknown)
            {
                info["char"] = *ch;
                user.send_numeric_reply(ERR_UNKNOWNMODE, info);
            }
        }
    }
    return mode_set;
}

std::string Channel::change_mode(chan_mode_set_t const &new_mode)
{
    unsigned short old_mode;
    std::string add_params;
    std::string rm_params;
    std::string add;
    std::string rm;

    old_mode = this->mode;
    this->mode = new_mode.mode;

    size_t arr_size = sizeof(supported_modes) / sizeof(chan_mode_map_t);
    for (size_t i = 0; i < arr_size; i++)
    {
        Channel::chan_mode_map_t mode_map = supported_modes[i];
        std::string value;
        switch (mode_map.mode_type)
        {
        case 'b':
            if (new_mode.type_b_params.count(mode_map.mode) == 0)
                break;
            for (std::map<std::string, std::stack<char> >::const_iterator it = new_mode.type_b_params.at(mode_map.mode).begin(); \
                it != new_mode.type_b_params.at(mode_map.mode).end(); it++)
            {
                if (it->second.top() == '+' && !is_type_b_param(mode_map.mode, it->first))
                {
                    add += mode_map.mode_char;
                    add_params += ' ' + it->first;
                    add_type_b_param(mode_map.mode, it->first);
                }
                else if (it->second.top() == '-' && is_type_b_param(mode_map.mode, it->first))
                {
                    rm += mode_map.mode_char;
                    rm_params += ' ' + it->first;
                    remove_type_b_param(mode_map.mode, it->first);
                }
            }
            break;
        
        case 'c':
            if (old_mode & mode_map.mode && ~new_mode.mode & mode_map.mode)
            {
                rm += mode_map.mode_char;
                set_type_c_param(mode_map.mode, "");
                break;
            }
            else if (new_mode.mode & mode_map.mode)
            {
                if (new_mode.type_c_params.count(mode_map.mode) == 0)
                    break;
                value = new_mode.type_c_params.at(mode_map.mode);
                if (get_type_c_param(mode_map.mode) != value)
                {
                    set_type_c_param(mode_map.mode, value);
                    add += mode_map.mode_char;
                    if (mode_map.mode != CHANNEL_KEY)
                        add_params += ' ' + value;
                }
            }
            break;
        
        case 'd':
            if (mode_map.mode & new_mode.mode && mode_map.mode & ~old_mode)
                add += mode_map.mode_char;
            else if (mode_map.mode & old_mode && mode_map.mode & ~new_mode.mode)
                rm += mode_map.mode_char;
            break;
        
        default:
            break;
        }
    }
    return (add.empty() ? add : '+' + add) + (rm.empty() ? rm : '-' + rm) + add_params + rm_params;
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

bool Channel::mode_str_has_enough_params(std::string const &mode_str, size_t param_count)
{
    char curr_sign = mode_str[0];

    size_t req_param_count = 0;
    for (std::string::const_iterator i = mode_str.begin(); i < mode_str.end(); i++)
    {
        if (*i == '+' || *i == '-')
        {
            curr_sign = *i;
            continue ;
        }
        if (mode_requires_param(*i, curr_sign))
            req_param_count += 1;
    }

    if (param_count < req_param_count)
        return false;
    return true;
}

bool Channel::mode_requires_param(char mode, char sign)
{
    char type;

    size_t arr_size = sizeof(supported_modes) / sizeof(chan_mode_map_t);
    for (size_t i = 0; i < arr_size; i++)
    {
        if (supported_modes[i].mode_char == mode)
        {
            type = supported_modes[i].mode_type;
            return type == 'a' || type == 'b' || (type == 'c' && sign == '+');
        }
    }
    return false;
}

void Channel::parse_type_b_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param)
{
    if (!mode_set.type_b_params[mode][param].empty())
    {
        if ((sign == '+' && mode_set.type_b_params[mode][param].top() == '-') || \
            (sign == '-' && mode_set.type_b_params[mode][param].top() == '+'))
            mode_set.type_b_params[mode][param].pop();
    }
    mode_set.type_b_params[mode][param].push(sign);
}

void Channel::parse_type_c_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param)
{
    if (sign == '+')
    {
        mode_set.mode |= mode;
        mode_set.type_c_params[mode] = param;
    }
    else
        mode_set.mode &= ~mode;
}

void Channel::parse_type_d_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign)
{
    if (sign == '+')
        mode_set.mode |= mode;
    else
        mode_set.mode &= ~mode;
}


// ============================
//       Sending messages
// ============================

void Channel::notify(std::string const &source, std::string const &cmd, std::string const &param) const
{
    std::string message;
    
    message = app.create_message(source, cmd, name + ' ' + param);
    for (std::vector<Client *>::const_iterator i = clients.begin(); i < clients.end(); i++)
        (*i)->send_message(message);
}

void Channel::privmsg(std::string const &source, std::string const &msg) const
{
    std::string message;
    
    message = app.create_message(source, "PRIVMSG", name + ' ' + msg);
    for (std::vector<Client *>::const_iterator i = clients.begin(); i < clients.end(); i++)
    {
        if ((*i)->get_full_nickname() == source)
            continue;
        (*i)->send_message(message);
    }
}