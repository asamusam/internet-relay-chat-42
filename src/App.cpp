#include "App.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>
#include <sys/socket.h>

// ============================
//   Constructor & Destructor
// ============================

App::App(std::string const &name, std::string const &password) : server_name(name), server_password(password)
{
    commands.push_back((Command){"PASS",    &App::pass});
    commands.push_back((Command){"NICK",    &App::nick});
    commands.push_back((Command){"USER",    &App::user});
    commands.push_back((Command){"JOIN",    &App::join});
    commands.push_back((Command){"PRIVMSG", &App::privmsg});
    commands.push_back((Command){"KICK",    &App::kick});
    commands.push_back((Command){"INVITE",  &App::invite});
    commands.push_back((Command){"TOPIC",   &App::topic});
    commands.push_back((Command){"MODE",    &App::mode});
}

App::~App()
{
    free_channels();
    free_clients();
}


// ============================
//      Memory management
// ============================

void App::free_clients(void)
{
    for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
        delete it->second;
}

void App::free_channels(void)
{
    for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++)
        delete it->second;
}


// ============================
//          Clients
// ============================

void App::add_client(Client *new_client)
{
    clients[new_client->uuid] = new_client;
}

void App::remove_client(int uuid)
{
    std::map<int, Client *>::iterator it;

    it = clients.find(uuid);
    if (it == clients.end())
        return ;
    delete it->second;
    clients.erase(uuid);
}


// ============================
//          Channels
// ============================

void App::add_channel(Channel *new_channel)
{
    channels[new_channel->name] = new_channel;
}


// ============================
//       Helper functions
// ============================

static void skip_space(std::istringstream &msg_stream)
{
    while (msg_stream.peek() == ' ')
        msg_stream.ignore(1);
}

static int split_targets(std::string const &target_str, std::vector<std::string> &targets)
{
    std::istringstream ss(target_str);
    std::string target;

    while (!ss.eof())
    {
        std::getline(ss, target, ',');
        if (!target.empty())
        {
            if (std::find(targets.begin(), targets.end(), target) != targets.end())
                return -1;
            targets.push_back(target);
        }
    }
    return 0;
}

static std::string int_to_string(int number)
{
    std::stringstream ss;
    ss << number;
    if (number < 10)
        return "00" + ss.str();
    return ss.str();
}

Client *App::find_client_by_nick(std::string const &nick) const
{
    for (std::map<int, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->nickname == nick)
            return i->second;
    }
    return NULL;
}

Client *App::find_client_by_fd(int fd) const
{
    for (std::map<int, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->fd == fd)
            return i->second;
    }
    return NULL;
}


// ============================
//       Sending messages
// ============================

void App::send_message(Client const &client, std::string const &message) const
{
    std::string crlf_msg;

    crlf_msg = message + CRLF;
	std::cout << "Send msg to uuid:" << client.uuid << " ->" << message << "\n";
	send(client.fd, crlf_msg.c_str(), crlf_msg.size(), 0);
}

void App::send_message_to_targets(Client const &user, std::string const &cmd, std::string const &msg, std::vector<std::string> const &targets) const
{
    Client *recipient;
    std::map<std::string, Channel *>::const_iterator channel;
    std::string message;
    std::map<std::string, std::string> info;

    message = ':' + user.nickname + ' ' + cmd + ' ' + msg;
    for (std::vector<std::string>::const_iterator i = targets.begin(); i < targets.end(); i++)
    {
        recipient = find_client_by_nick(*i);
        if (recipient && recipient->is_registered)
            send_message(*recipient, message);
        else
        {
            channel = channels.find(*i);
            if (channel != channels.end())
                send_message_to_targets(user, cmd, *i + ' ' + msg, channel->second->get_client_nicks());
            else
            {
                info["nick"] = *i;
                send_numeric_reply(user, ERR_NOSUCHNICK, info);
            }
        }
    }    
}

// ============================
//      Numeric replies
// ============================

void App::fill_placeholders(std::string &str, std::map<std::string, std::string> const &info) const
{
    std::map<std::string, std::string>::const_iterator it;
    std::size_t start_pos;
    std::size_t end_pos;
    std::string key;

    while (true)
    {
        start_pos = str.find('<');
        end_pos = str.find('>');
        if (start_pos == std::string::npos || end_pos == std::string::npos || end_pos < start_pos)
            break ;
        key = str.substr(start_pos + 1, end_pos - start_pos - 1);
        it = info.find(key);
        if (it != info.end())
            str.replace(start_pos, end_pos - start_pos + 1, it->second);
        else
            break ;
    }
}

void App::send_numeric_reply(Client const &client, IRCReplyCodeEnum code, std::map<std::string, std::string> const &info) const
{
    std::string reply_text;
    std::string msg;

    reply_text = IRCReply::get_reply_message(code);
    fill_placeholders(reply_text, info);
    msg = ':' + this->server_name + ' ' + int_to_string(code) + ' ' + reply_text;
    send_message(client, msg);
}

void App::send_numeric_reply(Client const &client, IRCReplyCodeEnum code, std::string const &msg) const
{
    std::string reply;

    reply = ':' + this->server_name + ' ' + int_to_string(code) + ' ' + msg;
    send_message(client, reply);
}


// ============================
//            PASS
// ============================

/*
If the message contains multiple parameters for PASS command,
only the first one is taken into consideration and the rest are ignored.

If the user sends an additional PASS command with incorrect password
after already setting nickname or username, the server replies with ERR_PASSWDMISMATCH,
and future attempts to complete the registration with NICK/USER commands lead
to the same result until the correct password is set.
*/
void App::pass(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;

    if (user.is_registered)
        return send_numeric_reply(user, ERR_ALREADYREGISTRED, info);
    if (params.empty())
    {
        info["command"] = "PASS";
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    }
    if (params[0] != this->server_password)
    {
        if (user.has_valid_pwd)
            user.has_valid_pwd = false;
        return send_numeric_reply(user, ERR_PASSWDMISMATCH, info);
    }
    user.has_valid_pwd = true;
}


// ============================
//          NICK
// ============================

/*
If the nickname is already used by other client,
the server just sends back ERR_NICKNAMEINUSE reply.
*/
void App::nick(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::string nickname;

    if (!user.has_valid_pwd)
        return send_numeric_reply(user, ERR_PASSWDMISMATCH, info);
    if (params.empty())
        return send_numeric_reply(user, ERR_NONICKNAMEGIVEN, info);
    nickname = params[0];
    info["nick"] = nickname;
    if (!is_valid_nick(nickname))
        return send_numeric_reply(user, ERR_ERRONEUSNICKNAME, info);
    if (find_client_by_nick(nickname))
        return send_numeric_reply(user, ERR_NICKNAMEINUSE, info);
    user.nickname = nickname;
    if (!user.is_registered && !user.username.empty())
    {
        user.is_registered = true;
        info["network"] = "42 London";
        info["client"] = user.nickname;
        send_numeric_reply(user, RPL_WELCOME, info);
    }
}

/*
Nickname has a maximum length of nine (9) characters.
<nick>>: <letter> { <letter> | <number> | <special> }
<letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
<number>     ::= '0' ... '9'
<special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
*/
bool App::is_valid_nick(std::string const &nickname) const
{
    size_t nick_len = nickname.length();
    std::string special("-[]\\`^{}");

    if (nick_len > this->nick_max_len)
        return false;
    if (!std::isalpha(nickname[0]))
        return false;
    for (size_t i = 0; i < nick_len; i++)
    {
        if (!std::isalnum(nickname[i]) && special.find(nickname[i]) == special.npos)
            return false;
    }
    return true;
}


// ============================
//          USER
// ============================

/*
Maximum length is 12. If the username exceeds this limit, it is silently truncated.
Expected format:
<user>     ::= <nonwhite> { <nonwhite> }
<nonwhite> ::= <any 8bit code except SPACE (0x20), NUL (0x0), CR (0xd), and LF (0xa)>
If the username doesn't comply with the rules described above, the message
is silently ignored by the server.
*/
void App::user(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::string username;

    if (user.is_registered)
        return send_numeric_reply(user, ERR_ALREADYREGISTRED, info);
    if (!user.has_valid_pwd)
        return send_numeric_reply(user, ERR_PASSWDMISMATCH, info);
    if (params.empty())
    {
        info["command"] = "USER";
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    }
    username = params[0];
    if (username.find(' ') != username.npos)
        return ;
    if (username.length() > 12)
        user.username = username.substr(0, 12);
    else
        user.username = username;
    if (!user.nickname.empty())
    {
        user.is_registered = true;
        info["nick"] = user.nickname;
        info["network"] = "42 London";
        info["client"] = user.nickname;
        send_numeric_reply(user, RPL_WELCOME, info);
        // send_numeric_reply(user, RPL_YOURHOST, info);
        // send_numeric_reply(user, RPL_CREATED, info);
        // send_numeric_reply(user, RPL_MYINFO, info);
        // send_numeric_reply(user, ERR_NOMOTD, info);
    }
    //std::cout << "New username: " << user.username 
    //          << (user.is_registered ? "\nRegistration complete." : "") << std::endl;
}


// ============================
//          JOIN
// ============================

/*
Parameters: <channel> [<key>]
*/
void App::join(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::map<std::string, Channel *>::iterator channel_it;
    Channel *channel;
    std::string channel_name;

    if (!user.is_registered)
        return ;
    info["command"] = "JOIN";
    if (params.empty())
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    if (params[0].length() > 200)
        channel_name = channel_name.substr(0, 200);
    else
        channel_name = params[0];
    info["channel"] = channel_name;
    if (user.num_channels == this->client_channel_limit)
        return send_numeric_reply(user, ERR_TOOMANYCHANNELS, info);
    channel_it = channels.find(channel_name);
    if (channel_it != channels.end())
    {
        channel = channel_it->second;
        if (channel->is_on_channel(user.nickname))
            return ;
        if (channel->is_in_mode(INVITE_ONLY) && !channel->is_invited(user.nickname))
            return send_numeric_reply(user, ERR_INVITEONLYCHAN, info);
        if (channel->is_full())
            return send_numeric_reply(user, ERR_CHANNELISFULL, info);
        if (channel->is_in_mode(CHANNEL_KEY))
        {
            if (params.size() < 2)
                return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
            if (!channel->is_matching_key(params[1]))
                return send_numeric_reply(user, ERR_BADCHANNELKEY, info);
        }
    }
    else
    {
        if (!is_valid_channel_name(channel_name))
            return send_numeric_reply(user, ERR_BADCHANMASK, info);
        channel = new Channel(channel_name);
        channel->add_type_b_param(CHAN_OP, user.nickname);
        this->channels[channel->name] = channel;
    }
    channel->add_client(user.nickname);
    send_message_to_targets(user, info["command"], info["channel"], channel->get_client_nicks());
    info["topic"] = channel->get_topic();
    info["nicks"] = channel->get_client_nicks_str();
    if (!info["topic"].empty())
        send_numeric_reply(user, RPL_TOPIC, info);
    send_numeric_reply(user, RPL_NAMREPLY, info);
}

/*
Channels names are strings (beginning with a '&' or '#' character) of
length up to 200 characters.  Apart from the the requirement that the
first character being either '&' or '#'; the only restriction on a
channel name is that it may not contain any spaces (' '), a control G
(^G or ASCII 7), or a comma (',' which is used as a list item
separator by the protocol).
*/
bool App::is_valid_channel_name(std::string const &channel_name) const
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
//           PRIVMSG
// ============================

/*
Parameters: PRIVMSG <receiver>{,<receiver>} <text to be sent>
<receiver> can be a nickname or a channel.
Message can be sent to the same client that sends it.
*/
void App::privmsg(Client &user, std::vector<std::string> const &params)
{
    std::vector<std::string> targets;
    std::map<std::string, std::string> info;
    std::string target;

    if (!user.is_registered)
        return ;
    info["command"] = "PRIVMSG";
    if (params.empty())
        return send_numeric_reply(user, ERR_NORECIPIENT, info);
    if (params.size() < 2)
        return send_numeric_reply(user, ERR_NOTEXTTOSEND, info);
    target = params[0];
    info["target"] = target;
    if (target.find(',') == target.npos)
        targets.push_back(params[0]);
    else if (split_targets(target, targets) == -1)
        return send_numeric_reply(user, ERR_TOOMANYTARGETS, info);

    send_message_to_targets(user, info["command"], params[1], targets);
}


// ============================
//           KICK
// ============================

/*
Parameters: <channel> <user> [<comment>]
*/
void App::kick(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::map<std::string, Channel *>::const_iterator channel_it;

    if (!user.is_registered)
        return ;
    info["command"] = "KICK";
    if (params.size() < 2)
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    info["channel"] = params[0];
    info["user"] = params[1];
    channel_it = channels.find(info["channel"]);
    if (channel_it == channels.end())
        return send_numeric_reply(user, ERR_NOSUCHCHANNEL, info);
    if (!channel_it->second->is_on_channel(user.nickname))
        return send_numeric_reply(user, ERR_NOTONCHANNEL, info);
    if (!channel_it->second->is_channel_operator(user.nickname))
        return send_numeric_reply(user, ERR_CHANOPRIVSNEEDED, info);
    if (!channel_it->second->is_on_channel(info["user"]))
        return send_numeric_reply(user, ERR_USERNOTINCHANNEL, info);
    send_message_to_targets(user, info["command"], info["channel"] + ' ' + info["user"], channel_it->second->get_client_nicks());
    channel_it->second->remove_client(info["user"]);
    if (channel_it->second->get_client_count() == 0)
    {
        delete channel_it->second;
        channels.erase(channel_it->first);
    }
}


// ============================
//           INVITE
// ============================

/*
Parameters: <nick> <channel>
*/
void App::invite(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::map<std::string, Channel *>::const_iterator channel_it;
    Client *recipient;
    std::string invitation_msg;

    if (!user.is_registered)
        return ;
    info["command"] = "INVITE";
    if (params.size() < 2)
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    info["nick"] = params[0];
    info["user"] = params[0];
    info["channel"] = params[1];
    recipient = find_client_by_nick(info["nick"]);
    if (!recipient)
        return send_numeric_reply(user, ERR_NOSUCHNICK, info);
    channel_it = channels.find(info["channel"]);
    if (channel_it == channels.end())
        return send_numeric_reply(user, ERR_NOSUCHCHANNEL, info);
    if (!channel_it->second->is_on_channel(user.nickname))
        return send_numeric_reply(user, ERR_NOTONCHANNEL, info);
    if (channel_it->second->is_in_mode(INVITE_ONLY) && !channel_it->second->is_channel_operator(user.nickname))
        return send_numeric_reply(user, ERR_CHANOPRIVSNEEDED, info);
    if (channel_it->second->is_on_channel(info["nick"]))
        return send_numeric_reply(user, ERR_USERONCHANNEL, info);
    channel_it->second->add_invitation(info["nick"]);
    invitation_msg = ':' + user.nickname + " INVITE " + info["nick"] + ' ' + info["channel"];
    send_message(*recipient, invitation_msg);
    send_numeric_reply(user, RPL_INVITING, info);
}


// ============================
//           TOPIC
// ============================

/*
Parameters: <channel> [<topic>]
*/
void App::topic(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::map<std::string, Channel *>::const_iterator channel_it;
    
    if (!user.is_registered)
        return ;
    info["command"] = "TOPIC";
    if (params.empty())
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    info["channel"] = params[0];
        channel_it = channels.find(info["channel"]);
    if (channel_it == channels.end())
        return send_numeric_reply(user, ERR_NOSUCHCHANNEL, info);
    if (!channel_it->second->is_on_channel(user.nickname))
        return send_numeric_reply(user, ERR_NOTONCHANNEL, info);
    if (params.size() == 1)
    {
        info["topic"] = channel_it->second->get_topic();
        if (info["topic"].empty())
            return send_numeric_reply(user, RPL_NOTOPIC, info);
        else
            return send_numeric_reply(user, RPL_TOPIC, info);
    }
    if (channel_it->second->is_in_mode(TOPIC_LOCK) && !channel_it->second->is_channel_operator(user.nickname))
        return send_numeric_reply(user, ERR_CHANOPRIVSNEEDED, info);
    info["topic"] = params[1];
    if (info["topic"] == ":")
        channel_it->second->set_topic("");
    else
        channel_it->second->set_topic(info["topic"]);
    send_message_to_targets(user, info["command"], info["channel"] + ' ' + info["topic"], channel_it->second->get_client_nicks());
}


// ============================
//           MODE
// ============================

void App::mode(Client &user, std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::map<std::string, Channel *>::const_iterator channel_it;
    chan_mode_set_t change_mode;
    std::string change_mode_str;
    Channel *channel;
    
    if (!user.is_registered)
        return ;

    info["command"] = "MODE";
    if (params.empty())
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    
    info["channel"] = params[0];
    channel_it = channels.find(info["channel"]);
    if (channel_it == channels.end())
        return send_numeric_reply(user, ERR_NOSUCHCHANNEL, info);
    
    channel = channel_it->second;
    if (!channel->is_on_channel(user.nickname))
        return send_numeric_reply(user, ERR_NOTONCHANNEL, info);

    // inform about the current channel mode
    if (params.size() < 2)
        return send_numeric_reply(user, RPL_CHANNELMODEIS, channel->get_mode_string(user.nickname));
    
    // change the channel mode
    if (!channel->is_channel_operator(user.nickname))
        return send_numeric_reply(user, ERR_CHANOPRIVSNEEDED, info);
    if (!mode_str_has_enough_params(params[1], params.size() - 2))
        return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
    if (params[1][0] != '+' && params[1][0] != '-')
    {
        info["char"] = params[1][0];
        return send_numeric_reply(user, ERR_UNKNOWNMODE, info);
    }
    change_mode = parse_mode_string(user, channel, params[1], params);
    change_mode_str = change_channel_mode(channel, change_mode);
    if (!change_mode_str.empty())
        send_message_to_targets(user, info["command"], info["channel"] + ' ' + change_mode_str, channel->get_client_nicks());
}

bool App::mode_str_has_enough_params(std::string const &mode_str, size_t param_count)
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

/*
Type A: MUST always have a parameter;
Type B: MUST always have a parameter;
Type C: MUST have a parameter when being set, and MUST NOT have a parameter when being unset;
Type D: MUST NOT have a parameter.
*/
bool App::mode_requires_param(char mode, char sign)
{
    char type;

    size_t arr_size = sizeof(Channel::supported_modes) / sizeof(chan_mode_map_t);
    for (size_t i = 0; i < arr_size; i++)
    {
        if (Channel::supported_modes[i].mode_char == mode)
        {
            type = Channel::supported_modes[i].mode_type;
            return type == 'a' || type == 'b' || (type == 'c' && sign == '+');
        }
    }
    return false;
}

chan_mode_set_t App::parse_mode_string(Client const &user, Channel const *channel, std::string const &mode_str, std::vector<std::string> const &params) const
{
    std::map<std::string, std::string> info;
    chan_mode_set_t mode_set;
    Client *target;

    mode_set.mode = channel->get_mode();
    mode_set.type_c_params[CHANNEL_KEY] = channel->get_key();
    mode_set.user_limit = channel->get_user_limit();
    info["channel"] = channel->name;

    char sign = mode_str[0];
    size_t index = 2;
    size_t supported_modes_size = sizeof(Channel::supported_modes) / sizeof(chan_mode_map_t);
    for (std::string::const_iterator ch = mode_str.begin(); ch < mode_str.end(); ch++)
    {
        if (*ch == '+' || *ch == '-')
        {
            sign = *ch;
            continue;
        }
        else
        {
            bool unknown = true;
            for (size_t i = 0; i < supported_modes_size; i++)
            {
                if (Channel::supported_modes[i].mode_char == *ch)
                {
                    switch (Channel::supported_modes[i].mode_type)
                    {
                        case 'b':
                            target = find_client_by_nick(params[index]);
                            info["nick"] = params[index++];
                            if (!target)
                                send_numeric_reply(user, ERR_NOSUCHNICK, info);
                            else if (!channel->is_on_channel(target->nickname))
                                send_numeric_reply(user, ERR_NOTONCHANNEL, info);
                            else
                                parse_type_b_mode(mode_set, Channel::supported_modes[i].mode, sign, target->nickname);
                            break;
                        
                        case 'c':
                            if (*ch == 'k' && sign == '+' && mode_set.mode & CHANNEL_KEY)
                                send_numeric_reply(user, ERR_KEYSET, info);
                            else
                                parse_type_c_mode(mode_set, Channel::supported_modes[i].mode, sign, params[index]);
                            if (sign == '+')
                                ++index;
                            break;
                        
                        case 'd':
                            parse_type_d_mode(mode_set, Channel::supported_modes[i].mode, sign);
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
                send_numeric_reply(user, ERR_UNKNOWNMODE, info);
            }
        }
    }
    return mode_set;
}

void App::parse_type_b_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param) const
{
    if (!mode_set.type_b_params[mode][param].empty())
    {
        if ((sign == '+' && mode_set.type_b_params[mode][param].top() == '-') || \
            (sign == '-' && mode_set.type_b_params[mode][param].top() == '+'))
            mode_set.type_b_params[mode][param].pop();
    }
    mode_set.type_b_params[mode][param].push(sign);
}

void App::parse_type_c_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign, std::string const &param)
{
    if (sign == '+')
    {
        mode_set.mode |= mode;
        mode_set.type_c_params[mode] = param;
    }
    else
        mode_set.mode &= ~mode;
}

void App::parse_type_d_mode(chan_mode_set_t &mode_set, chan_mode_enum mode, char sign)
{
    if (sign == '+')
        mode_set.mode |= mode;
    else
        mode_set.mode &= ~mode;
}

std::string App::change_channel_mode(Channel *channel, chan_mode_set_t const &new_mode)
{
    unsigned short old_mode;
    std::string add_params;
    std::string rm_params;
    std::string add;
    std::string rm;

    old_mode = channel->get_mode();
    channel->set_mode(new_mode.mode);

    size_t arr_size = sizeof(Channel::supported_modes) / sizeof(chan_mode_map_t);
    for (size_t i = 0; i < arr_size; i++)
    {
        chan_mode_map_t mode_map = Channel::supported_modes[i];
        std::string value;
        switch (mode_map.mode_type)
        {
        case 'b':
            if (new_mode.type_b_params.count(mode_map.mode) == 0)
                break;
            for (std::map<std::string, std::stack<char> >::const_iterator it = new_mode.type_b_params.at(mode_map.mode).begin(); \
                it != new_mode.type_b_params.at(mode_map.mode).end(); it++)
            {
                if (it->second.top() == '+' && !channel->is_type_b_param(mode_map.mode, it->first))
                {
                    add += mode_map.mode_char;
                    add_params += ' ' + it->first;
                    channel->add_type_b_param(mode_map.mode, it->first);
                }
                else if (it->second.top() == '-' && channel->is_type_b_param(mode_map.mode, it->first))
                {
                    rm += mode_map.mode_char;
                    rm_params += ' ' + it->first;
                    channel->remove_type_b_param(mode_map.mode, it->first);
                }
            }
            break;
        
        case 'c':
            if (old_mode & mode_map.mode && ~new_mode.mode & mode_map.mode)
            {
                rm += mode_map.mode_char;
                channel->set_type_c_param(mode_map.mode, "");
                break;
            }
            else if (new_mode.mode & mode_map.mode)
            {
                if (new_mode.type_c_params.count(mode_map.mode) == 0)
                    break;
                value = new_mode.type_c_params.at(mode_map.mode);
                if (channel->get_type_c_param(mode_map.mode) != value)
                {
                    channel->set_type_c_param(mode_map.mode, value);
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


// ============================
//         Execution
// ============================

/*
Runs the command and sends appropriate replies.
*/
void App::execute_message(Client &user, Message const &msg)
{
    std::map<std::string, std::string> info;
    
    for (std::vector<Command>::const_iterator i = commands.begin(); i < commands.end(); i++)
    {
        if (i->name == msg.command)
        {
            (this->*(i->cmd_func))(user, msg.params);
            return;
        }
    }
    info["command"] = msg.command;
    send_numeric_reply(user, ERR_UNKNOWNCOMMAND, info);
}


// ============================
//          Parsing
// ============================

/*
On success, returns 0 and fills the Message structure provided.
On error, returns -1, which means the message should be ignored.
*/
int App::parse_message(Client &user, std::string const &msg_string, Message &msg) const
{
    std::istringstream msg_stream(msg_string);
    std::string param;

    if (msg_stream.peek() == ':')
    {
        msg_stream.ignore(1);
        std::getline(msg_stream, msg.prefix, ' ');
        if (!user.is_registered || user.nickname != msg.prefix)
            return -1;
        skip_space(msg_stream);
    }

    std::getline(msg_stream, msg.command, ' ');
    skip_space(msg_stream);

    while (!msg_stream.eof())
    {
        if (msg_stream.peek() == ':')
        {
            msg_stream.ignore(1);
            std::getline(msg_stream, param);
        }
        else
            std::getline(msg_stream, param, ' ');
        if (!param.empty())
        {
            // std::cout << "new param: " << param << std::endl;
            msg.params.push_back(param);
        }
        skip_space(msg_stream);
    }

    return 0;
}
