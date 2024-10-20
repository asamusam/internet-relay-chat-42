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

    error_messages[ERR_UNKNOWNCOMMAND]   = "<command> :Unknown command";
    error_messages[ERR_NEEDMOREPARAMS]   = "<command> :Not enough parameters";
    error_messages[ERR_ALREADYREGISTRED] = ":You may not reregister";
    error_messages[ERR_PASSWDMISMATCH]   = ":Password incorrect";
    error_messages[ERR_NICKNAMEINUSE]    = "<nick> :Nickname is already in use";
    error_messages[ERR_NONICKNAMEGIVEN]  = ":No nickname given";
    error_messages[ERR_ERRONEUSNICKNAME] = "<nick> :Erroneus nickname";
    error_messages[ERR_NORECIPIENT]      = ":No recipient given (<command>)";
    error_messages[ERR_NOTEXTTOSEND]     = ":No text to send";
    error_messages[ERR_CANNOTSENDTOCHAN] = "<channel name> :Cannot send to channel";
    error_messages[ERR_TOOMANYTARGETS]   = "<target> :Duplicate recipients. No message delivered";
    error_messages[ERR_NOSUCHNICK]       = "<nickname> :No such nick/channel";
    error_messages[ERR_NOSUCHCHANNEL]    = "<channel> :No such channel";
    error_messages[ERR_TOOMANYCHANNELS]  = "<channel> :You have joined too many channels";
    error_messages[ERR_INVITEONLYCHAN]   = "<channel> :Cannot join channel (invite only)";
    error_messages[ERR_BANNEDFROMCHAN]   = "<channel> :Cannot join channel (banned)";
    error_messages[ERR_CHANNELISFULL]    = "<channel> :Cannot join channel (channel is full)";
    error_messages[ERR_BADCHANNELKEY]    = "<channel> :Cannot join channel (incorrect channel key)";
    error_messages[ERR_USERONCHANNEL]    = "<user> <channel> :is already on channel";
    error_messages[ERR_USERNOTINCHANNEL] = "<user> <channel> :They are not on that channel";
    error_messages[ERR_NOTONCHANNEL]     = "<channel> :You're not on that channel";
    error_messages[ERR_CHANOPRIVSNEEDED] = "<channel> :You're not channel operator";
    error_messages[ERR_KEYSET]           = "<channel> :Channel key already set";
    error_messages[ERR_UNKNOWNMODE]      = "<char> :is unknown mode char to me for <channel>";
    error_messages[ERR_NOPRIVILEGES]     = ":Permission Denied- You're not an IRC operator";
    error_messages[ERR_USERSDONTMATCH]   = ":Cannot change mode for other users";
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
    if (user.is_registered)
    {
        send_numeric_reply(user, ERR_ALREADYREGISTRED, NULL);
        return ;
    }
    if (params.empty())
    {
        send_numeric_reply(user, ERR_NEEDMOREPARAMS, "PASS");
        return ;
    }
    if (params[0] != this->server_password)
    {
        if (user.has_valid_pwd)
            user.has_valid_pwd = false;
        send_numeric_reply(user, ERR_PASSWDMISMATCH, NULL);
        return ;
    }
    user.has_valid_pwd = true;
}

/*
If the nickname is already used by other client,
the server just sends back ERR_NICKNAMEINUSE reply.
*/
void App::nick(Client &user, std::vector<std::string> const &params)
{
    if (!user.has_valid_pwd)
    {
        send_numeric_reply(user, ERR_PASSWDMISMATCH, NULL);
        return ;
    }
    if (params.empty())
    {
        send_numeric_reply(user, ERR_NONICKNAMEGIVEN, NULL);
        return ;
    }
    if (!nick_is_valid(params[0]))
    {
        send_numeric_reply(user, ERR_ERRONEUSNICKNAME, params[0].c_str());
        return ;
    }
    if (find_client_by_nick(params[0]))
    {
        send_numeric_reply(user, ERR_NICKNAMEINUSE, params[0].c_str());
        return ;
    }
    user.nickname = params[0];
    if (!user.is_registered && !user.username.empty())
        user.is_registered = true;
    //std::cout << "New nickname: " << user.nickname 
    //          << (user.is_registered ? "\nRegistration complete." : "") << std::endl;
}

/*
Nickname has a maximum length of nine (9) characters.
<nick>>: <letter> { <letter> | <number> | <special> }
<letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
<number>     ::= '0' ... '9'
<special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
*/
bool App::nick_is_valid(std::string const &nickname) const
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
    if (user.is_registered)
    {
        send_numeric_reply(user, ERR_ALREADYREGISTRED, NULL);
        return ;
    }
    if (!user.has_valid_pwd)
    {
        send_numeric_reply(user, ERR_PASSWDMISMATCH, NULL);
        return ;
    }
    if (params.empty())
    {
        send_numeric_reply(user, ERR_NEEDMOREPARAMS, "USER");
        return ;
    }
    if (params[0].find(' ') != params[0].npos)
    {
        return ;
    }
    if (params[0].length() > 12)
        user.username = params[0].substr(0, 12);
    else
        user.username = params[0];
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
    std::map<std::string, Channel *>::iterator channel_it;
    Channel *channel;

    if (!user.is_registered)
        return ;
    if (params.empty())
    {
        send_numeric_reply(user, ERR_NEEDMOREPARAMS, "JOIN");
        return ;
    }
    if (user.num_channels == this->client_channel_limit)
    {
        send_numeric_reply(user, ERR_TOOMANYCHANNELS, params[0].c_str());
        return ;
    }
    channel_it = channels.find(params[0]);
    if (channel_it != channels.end())
    {
        channel = channel_it->second;
        if (channel->has_user(user.nickname))
            return ;
        if (channel->is_invite_only() && !channel->is_invited_user(user.nickname))
        {
            send_numeric_reply(user, ERR_INVITEONLYCHAN, channel->name.c_str());
            return ;
        }
        if (channel->is_full())
        {
            send_numeric_reply(user, ERR_CHANNELISFULL, channel->name.c_str());
            return ;
        }
        if (channel->is_key_protected())
        {
            if (params.size() < 2)
            {
                send_numeric_reply(user, ERR_NEEDMOREPARAMS, "JOIN");
                return ;
            }
            if (!channel->is_matching_key(params[1]))
            {
                send_numeric_reply(user, ERR_BADCHANNELKEY, channel->name.c_str());
                return ;
            }
        }
        channel->add_client(user.nickname);
        std::cout << user.nickname << " joined " << channel->name << std::endl;
        // send RPL_TOPIC
        // send RPL_NAMREPLY
    }
    else
    {
        // try create a channel
            // if name is invalid
                // ERR_BADCHANMASK
        // add channel to the list of channels
        // add user to the channel
        // send RPL_TOPIC
        // send RPL_NAMREPLY
        // make the user channel operator
    }
}

// TODO: make a template
int split_targets(std::string const &target_str, std::vector<std::string> &targets)
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
            std::cout << "new target: " << target << std::endl;
        }
    }
    return 0;
}

void App::send_msg_to_targets(Client const &user, std::string const &msg, std::vector<std::string> const &targets) const
{
    Client *recipient;
    std::map<std::string, Channel *>::const_iterator channel;
    std::string message;

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
                send_msg_to_targets(user, *i + ' ' + msg, channel->second->get_clients());
            else
                send_numeric_reply(user, ERR_NOSUCHNICK, (*i).c_str());
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

    if (!user.is_registered)
        return ;
    if (params.empty())
    {
        send_numeric_reply(user, ERR_NORECIPIENT, "PRIVMSG");
        return ;
    }
    if (params.size() < 2)
    {
        send_numeric_reply(user, ERR_NOTEXTTOSEND, NULL);
        return ;
    }
    if (params[0].find(',') != params[0].npos)
    {
        if (split_targets(params[0], targets) == -1)
        {
            send_numeric_reply(user, ERR_TOOMANYTARGETS, params[0].c_str());
            return ;
        }
    }
    else
        targets.push_back(params[0]);

    send_msg_to_targets(user, params[1], targets);
}

void App::kick(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return ;
}

/*
*/
void App::invite(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return ;
}

void App::topic(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return ;
}

void App::mode(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return ;
}

// TODO: multiple vars and smarter substitution
void App::send_numeric_reply(Client const &user, int err, char const *var) const
{
    std::string err_msg;
    std::size_t start_pos;
    std::size_t end_pos;
    std::string reply;

    err_msg = error_messages.find(err)->second;
    start_pos = err_msg.find('<');
    end_pos = err_msg.find('>');

    if (start_pos != std::string::npos && end_pos != std::string::npos && end_pos > start_pos)
        err_msg.replace(start_pos, end_pos - start_pos + 1, var);

    reply = ':' + this->server_name + ' ' + int_to_string(err) + ' ' + err_msg;
    send_message(user, reply);
}

/*
Runs the command and sends appropriate replies.
*/
void App::run_message(Client &user, Message const &msg)
{
    std::string reply;
    
    for (std::vector<Command>::const_iterator i = commands.begin(); i < commands.end(); i++)
    {
        if (i->name == msg.command)
        {
            (this->*(i->cmd_func))(user, msg.params);
            return;
        }
    }
    send_numeric_reply(user, ERR_UNKNOWNCOMMAND, msg.command.c_str());
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
