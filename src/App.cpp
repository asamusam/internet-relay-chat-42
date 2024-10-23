#include "App.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>

static std::string int_to_string(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

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

void App::add_client(Client *new_client)
{
    clients[new_client->uuid] = new_client;
}

void App::add_channel(Channel *new_channel)
{
    channels[new_channel->name] = new_channel;
}

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
        user.is_registered = true;
    //std::cout << "New nickname: " << user.nickname 
    //          << (user.is_registered ? "\nRegistration complete." : "") << std::endl;
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

Client *App::find_client_by_nick(std::string const &nick) const
{
    for (std::map<int, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->nickname == nick)
            return i->second;
    }
    return NULL;
}

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
        user.is_registered = true;
    //std::cout << "New username: " << user.username 
    //          << (user.is_registered ? "\nRegistration complete." : "") << std::endl;
}

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
        if (channel->is_invite_only() && !channel->is_invited(user.nickname))
            return send_numeric_reply(user, ERR_INVITEONLYCHAN, info);
        if (channel->is_full())
            return send_numeric_reply(user, ERR_CHANNELISFULL, info);
        if (channel->is_key_protected())
        {
            if (params.size() < 2)
                return send_numeric_reply(user, ERR_NEEDMOREPARAMS, info);
            if (!channel->is_matching_key(params[1]))
                return send_numeric_reply(user, ERR_BADCHANNELKEY, info);
        }
        channel->add_client(user.nickname);
        info["topic"] = channel->get_topic();
        info["nicks"] = channel->get_client_nicks_str();
        send_numeric_reply(user, RPL_TOPIC, info);
        send_numeric_reply(user, RPL_NAMREPLY, info);
    }
    else
    {
        if (!is_valid_channel_name(channel_name))
            return send_numeric_reply(user, ERR_BADCHANMASK, info);
        channel = new Channel(channel_name);
        this->channels[channel->name] = channel;
        channel->add_client(user.nickname);
        channel->add_operator(user.nickname);
        info["topic"] = channel->get_topic();
        info["nicks"] = channel->get_client_nicks_str();
        send_numeric_reply(user, RPL_TOPIC, info);
        send_numeric_reply(user, RPL_NAMREPLY, info);
    }
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

void App::send_msg_to_targets(Client const &user, std::string const &msg, std::vector<std::string> const &targets) const
{
    Client *recipient;
    std::map<std::string, Channel *>::const_iterator channel;
    std::string message;
    std::map<std::string, std::string> info;

    for (std::vector<std::string>::const_iterator i = targets.begin(); i < targets.end(); i++)
    {
        recipient = find_client_by_nick(*i);
        if (recipient && recipient->is_registered)
        {
            message = ':' + user.nickname + " PRIVMSG " + msg;
            send_message(*recipient, message);
        }
        else
        {
            channel = channels.find(*i);
            if (channel != channels.end())
                send_msg_to_targets(user, *i + ' ' + msg, channel->second->get_client_nicks());
            else
            {
                info["nick"] = *i;
                send_numeric_reply(user, ERR_NOSUCHNICK, info);
            }
        }
    }    
}

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
    if (params.empty())
    {
        info["command"] = "PRIVMSG";
        return send_numeric_reply(user, ERR_NORECIPIENT, info);
    }
    if (params.size() < 2)
        return send_numeric_reply(user, ERR_NOTEXTTOSEND, info);

    target = params[0];
    info["target"] = target;
    if (target.find(',') == target.npos)
        targets.push_back(params[0]);
    else if (split_targets(target, targets) == -1)
        return send_numeric_reply(user, ERR_TOOMANYTARGETS, info);

    send_msg_to_targets(user, params[1], targets);
}

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
    channel_it->second->remove_client(info["user"]);
    if (channel_it->second->get_client_count() == 0)
    {
        delete channel_it->second;
        channels.erase(channel_it->first);
    }
}

/*
Parameters: <nick> <channel>
*/
void App::invite(Client &user, std::vector<std::string> const &params)
{
    (void)user;
    (void)params;
    
    if (!user.is_registered)
        return ;
}

void App::topic(Client &user, std::vector<std::string> const &params)
{
    (void)user;
    (void)params;
    
    if (!user.is_registered)
        return ;
}

void App::mode(Client &user, std::vector<std::string> const &params)
{
    (void)user;
    (void)params;
    
    if (!user.is_registered)
        return ;
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

void App::send_numeric_reply(Client const &user, IRCReplyCodeEnum code, std::map<std::string, std::string> const &info) const
{
    std::string reply_text;
    std::size_t start_pos;
    std::size_t end_pos;
    std::string key;
    std::map<std::string, std::string>::const_iterator it;
    std::string msg;

    reply_text = IRCReply::get_reply_message(code); // reply_texts.find(code)->second;
    while (true)
    {
        start_pos = reply_text.find('<');
        end_pos = reply_text.find('>');
        if (start_pos == std::string::npos || end_pos == std::string::npos || end_pos < start_pos)
            break ;
        key = reply_text.substr(start_pos + 1, end_pos - start_pos - 1);
        it = info.find(key);
        if (it != info.end())
            reply_text.replace(start_pos, end_pos - start_pos + 1, it->second);
        else
            break ;
    }
    msg = ':' + this->server_name + ' ' + int_to_string(code) + ' ' + reply_text;
    send_message(user, msg);
}

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


int App::send_message(Client const &client, std::string const &message) const
{
    std::cout << "Message sent to " << (client.nickname.empty() ? "client" : client.nickname) << '\n' << message << std::endl;
    return 0;
}

static void skip_space(std::istringstream &msg_stream)
{
    while (msg_stream.peek() == ' ')
        msg_stream.ignore(1);
}

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
            std::getline(msg_stream, param);
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
