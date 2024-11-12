#include "Client.hpp"
#include "Channel.hpp"

#include <ctime>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <algorithm>


// ============================
//   Constructor & Destructor
// ============================

Client::Client(App &app, int fd) : app(app), fd(fd), is_registered(false), has_valid_pwd(false), num_channels(0)
{
    uuid = generate_uuid();
}

Client::~Client() {}


// ============================
//         Getters
// ============================

std::string Client::get_nickname(void) const 
{
    return nickname;
}

int Client::get_fd(void) const
{
    return fd;
}

std::string Client::get_full_nickname() const
{
    return full_nickname;
}

uint32 Client::get_uuid() const
{
    return uuid;
}

std::string Client::get_msg_buff(void) const
{
	return msg_buff;
}

// ============================
//         Setters
// ============================

void Client::set_msg_buff(std::string const &s)
{
	msg_buff = s;
}

// ============================
//         UUID
// ============================

/*
As the upper limit of connections is WELL below MAX_INT there is little chance of conflict.
 */
uint32 Client::generate_uuid(void) const
{
	uint32 candidate;

	for (;;)
	{
		srand(time(0));
		candidate = (uint32) rand();
        if (app.get_client(candidate) == NULL)
            return candidate;
	}
}


// ============================
//         Registration
// ============================

void Client::register_client(void)
{
    std::map<std::string, std::string> info;

    this->is_registered = true;
    this->full_nickname = this->nickname + '!' + this->username + '@' + app.server_name;

    info["nick"] = nickname;
    info["client"] = full_nickname;
    info["network"] = app.network_name;
    info["servername"] = app.server_name;
    info["version"] = app.server_version;
    info["datetime"] = app.created_at;

    send_numeric_reply(RPL_WELCOME, info);
    send_numeric_reply(RPL_YOURHOST, info);
    send_numeric_reply(RPL_CREATED, info);
    // send_numeric_reply(user, RPL_MYINFO, info);
    // send_numeric_reply(user, ERR_NOMOTD, info);
}


// ============================
//         Checkers
// ============================

bool Client::is_registered_client(void) const
{
    return is_registered;
}

/*
Nickname has a maximum length of nine (9) characters.
<nick>>: <letter> { <letter> | <number> | <special> }
<letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
<number>     ::= '0' ... '9'
<special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
*/
bool Client::is_valid_nick(std::string const &nickname) const
{
    size_t nick_len = nickname.length();
    std::string special("-[]\\`^{}");

    if (nick_len > app.nick_max_len)
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
//  Sending messages & replies
// ============================

void Client::send_message(std::string const &message) const
{
    std::string crlf_msg;

    crlf_msg = message + CRLF;
	std::cout << "SEND msg to uuid:" << this->uuid << " ->" << message << "\n";
	send(this->fd, crlf_msg.c_str(), crlf_msg.size(), 0);
}

void Client::fill_placeholders(std::string &str, std::map<std::string, std::string> const &info)
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
            str.replace(start_pos, end_pos - start_pos + 1, "");
    }
}

void Client::send_numeric_reply(IRCReplyCodeEnum code, std::map<std::string, std::string> const &info) const
{
    std::string reply_text;
    std::string msg;

    reply_text = IRCReply::get_reply_message(code);
    fill_placeholders(reply_text, info);

    msg = app.create_message(app.server_name, IRCReply::code_to_string(code), reply_text);
    send_message(msg);
}

void Client::privmsg_client(std::string const &source, std::string const &msg) const
{
    std::string message;
    
    if (source == this->nickname)
        return;
    
    message = app.create_message(source, "PRIVMSG", this->nickname + ' ' + msg);
    this->send_message(message);
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
void Client::pass(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;

    info["client"] = full_nickname;
    info["command"] = "PASS";
    if (is_registered)
        return send_numeric_reply(ERR_ALREADYREGISTERED, info);
    if (params.empty())
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    if (!app.is_correct_pwd(params[0]))
    {
        if (has_valid_pwd)
            has_valid_pwd = false;
        return send_numeric_reply(ERR_PASSWDMISMATCH, info);
    }
    has_valid_pwd = true;
}


// ============================
//          NICK
// ============================

/*
If the nickname is already used by other client,
the server just sends back ERR_NICKNAMEINUSE reply.
*/
void Client::nick(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;

    info["client"] = this->nickname;
    if (!this->has_valid_pwd)
        return this->send_numeric_reply(ERR_PASSWDMISMATCH, info);
    if (params.empty())
        return this->send_numeric_reply(ERR_NONICKNAMEGIVEN, info);
    info["nick"] = params[0];
    if (!this->is_valid_nick(info["nick"]))
        return this->send_numeric_reply(ERR_ERRONEUSNICKNAME, info);
    if (app.find_client_by_nick(info["nick"]))
        return this->send_numeric_reply(ERR_NICKNAMEINUSE, info);
    this->nickname = info["nick"];
    if (!this->is_registered && !this->username.empty())
        this->register_client();
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
void Client::user(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    std::string username;

    info["client"] = this->nickname;
    info["command"] = "USER";
    if (this->is_registered)
        return send_numeric_reply(ERR_ALREADYREGISTERED, info);
    if (!this->has_valid_pwd)
        return send_numeric_reply(ERR_PASSWDMISMATCH, info);
    if (params.empty())
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    username = params[0];
    if (username.find(' ') != username.npos)
        return ;
    if (username.length() > 12)
        this->username = username.substr(0, 12);
    else
        this->username = username;
    if (!this->nickname.empty())
        this->register_client();
}


// ============================
//          JOIN
// ============================

/*
Parameters: <channel> [<key>]
*/
void Client::join(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    Channel *channel;

    if (!this->is_registered)
        return ;
    info["client"] = this->full_nickname;
    info["command"] = "JOIN";
    if (params.empty())
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    if (params[0].length() > 200)
        info["channel"] = params[0].substr(0, 200);
    else
        info["channel"] = params[0];
    if (this->num_channels == app.client_channel_limit)
        return send_numeric_reply(ERR_TOOMANYCHANNELS, info);
    channel = app.find_channel_by_name(info["channel"]);
    if (channel)
    {
        if (channel->is_on_channel(this))
            return ;
        if (channel->is_in_mode(INVITE_ONLY) && !channel->is_invited(this))
            return send_numeric_reply(ERR_INVITEONLYCHAN, info);
        if (channel->is_full())
            return send_numeric_reply(ERR_CHANNELISFULL, info);
        if (channel->is_in_mode(CHANNEL_KEY))
        {
            if (params.size() < 2)
                return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
            if (!channel->is_matching_key(params[1]))
                return send_numeric_reply(ERR_BADCHANNELKEY, info);
        }
    }
    else
    {
        if (!Channel::is_valid_channel_name(info["channel"]))
            return send_numeric_reply(ERR_BADCHANMASK, info);
        channel = app.create_channel(this->nickname, info["channel"]);
        app.add_channel(channel);
    }
    channel->add_client(this);
    channel->notify(this->full_nickname, info["command"], "");
    info["topic"] = channel->get_topic();
    info["nicks"] = channel->get_client_nicks_str();
    if (info["topic"] != ":")
        send_numeric_reply(RPL_TOPIC, info);
    info["symbol"] = "=";
    send_numeric_reply(RPL_NAMREPLY, info);
}


// ============================
//           PRIVMSG
// ============================

/*
Parameters: PRIVMSG <receiver>{,<receiver>} <text to be sent>
<receiver> can be a nickname or a channel.
Message can be sent to the same client that sends it.
*/
void Client::privmsg(std::vector<std::string> const &params)
{
    std::vector<std::string> targets;
    std::map<std::string, std::string> info;
    std::string target;

    if (!this->is_registered)
        return ;
    info["client"] = this->full_nickname;
    info["command"] = "PRIVMSG";
    if (params.empty())
        return send_numeric_reply(ERR_NORECIPIENT, info);
    if (params.size() < 2)
        return send_numeric_reply(ERR_NOTEXTTOSEND, info);
    target = params[0];
    info["target"] = target;
    if (target.find(',') == target.npos)
        targets.push_back(params[0]);
    else if (split_targets(target, targets) == -1)
        return send_numeric_reply(ERR_TOOMANYTARGETS, info);

    privmsg_targets(params[1], targets);
}

int Client::split_targets(std::string const &target_str, std::vector<std::string> &targets)
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

void Client::privmsg_targets(std::string const &msg, std::vector<std::string> const &targets) const
{
    std::map<std::string, std::string> info;
    Client *client;
    Channel *channel;

    for (std::vector<std::string>::const_iterator target = targets.begin(); target < targets.end(); target++)
    {
        client = app.find_client_by_nick(*target);
        if (client && client->is_registered)
            client->privmsg_client(this->get_full_nickname(), msg);
        else
        {
            channel = app.find_channel_by_name(*target);
            if (channel && channel->is_on_channel(this))
                channel->privmsg(this->full_nickname, msg);
            else if (channel)
            {
                info["channel"] = *target;
                send_numeric_reply(ERR_NOTONCHANNEL, info);
            }
            else
            {
                info["nick"] = *target;
                send_numeric_reply(ERR_NOSUCHNICK, info);
            }
        }
    }    
}


// ============================
//           KICK
// ============================

/*
Parameters: <channel> <user> [<comment>]
*/
void Client::kick(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    Channel *channel;
    Client *user;

    if (!is_registered)
        return ;
    info["client"] = this->full_nickname;
    info["command"] = "KICK";
    if (params.size() < 2)
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    info["channel"] = params[0];
    info["user"] = params[1];
    channel = app.find_channel_by_name(info["channel"]);
    if (!channel)
        return send_numeric_reply(ERR_NOSUCHCHANNEL, info);
    if (!channel->is_on_channel(this))
        return send_numeric_reply(ERR_NOTONCHANNEL, info);
    if (!channel->is_channel_operator(this->nickname))
        return send_numeric_reply(ERR_CHANOPRIVSNEEDED, info);
    user = app.find_client_by_nick(info["user"]);
    info["nick"] = info["user"];
    if (!user)
        return send_numeric_reply(ERR_NOSUCHNICK, info);
    if (!channel->is_on_channel(user))
        return send_numeric_reply(ERR_USERNOTINCHANNEL, info);
    channel->notify(this->full_nickname, info["command"], info["user"]);
    channel->remove_client(user);
}


// ============================
//           INVITE
// ============================

/*
Parameters: <nick> <channel>
*/
void Client::invite(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    Channel *channel;
    Client *recipient;
    std::string invite_msg;

    if (!this->is_registered)
        return ;
    info["client"] = this->full_nickname;
    info["command"] = "INVITE";
    if (params.size() < 2)
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    info["nick"] = params[0];
    info["user"] = params[0];
    info["channel"] = params[1];
    recipient = app.find_client_by_nick(info["nick"]);
    if (!recipient)
        return send_numeric_reply(ERR_NOSUCHNICK, info);
    channel = app.find_channel_by_name(info["channel"]);
    if (!channel)
        return send_numeric_reply(ERR_NOSUCHCHANNEL, info);
    if (!channel->is_on_channel(this))
        return send_numeric_reply(ERR_NOTONCHANNEL, info);
    if (channel->is_in_mode(INVITE_ONLY) && !channel->is_channel_operator(this->nickname))
        return send_numeric_reply(ERR_CHANOPRIVSNEEDED, info);
    if (channel->is_on_channel(recipient))
        return send_numeric_reply(ERR_USERONCHANNEL, info);
    channel->add_invite(recipient);
    invite_msg = app.create_message(this->full_nickname, info["command"], info["nick"] + ' ' + info["channel"]);
    recipient->send_message(invite_msg);
    send_numeric_reply(RPL_INVITING, info);
}


// ============================
//           TOPIC
// ============================

/*
Parameters: <channel> [<topic>]
*/
void Client::topic(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    Channel *channel;
    
    if (!this->is_registered)
        return ;
    info["client"] = this->full_nickname;
    info["command"] = "TOPIC";
    if (params.empty())
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    info["channel"] = params[0];
    channel = app.find_channel_by_name(info["channel"]);
    if (!channel)
        return send_numeric_reply(ERR_NOSUCHCHANNEL, info);
    if (!channel->is_on_channel(this))
        return send_numeric_reply(ERR_NOTONCHANNEL, info);
    if (params.size() == 1)
    {
        info["topic"] = channel->get_topic();
        if (info["topic"] == ":")
            return send_numeric_reply(RPL_NOTOPIC, info);
        else
            return send_numeric_reply(RPL_TOPIC, info);
    }
    if (channel->is_in_mode(TOPIC_LOCK) && !channel->is_channel_operator(this->nickname))
        return send_numeric_reply(ERR_CHANOPRIVSNEEDED, info);
    info["topic"] = params[1];
        channel->set_topic(info["topic"]);
    channel->notify(this->full_nickname, info["command"], info["topic"]);
}


// ============================
//           MODE
// ============================

void Client::mode(std::vector<std::string> const &params)
{
    std::map<std::string, std::string> info;
    Channel::chan_mode_set_t change_mode;
    std::string change_mode_str;
    Channel *channel;
    
    if (!this->is_registered)
        return ;

    info["client"] = this->full_nickname;
    info["command"] = "MODE";
    if (params.empty())
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    
    info["channel"] = params[0];
    if (info["channel"] == this->nickname)
        return ;
    channel = app.find_channel_by_name(info["channel"]);
    if (!channel)
        return send_numeric_reply(ERR_NOSUCHCHANNEL, info);
    if (!channel->is_on_channel(this))
        return send_numeric_reply(ERR_NOTONCHANNEL, info);

    // inform about the current channel mode
    if (params.size() < 2)
    {
        channel->get_mode_with_params(this->nickname, info);
        return send_numeric_reply(RPL_CHANNELMODEIS, info);
    }
    
    // change the channel mode
    if (!channel->is_channel_operator(this->nickname))
        return send_numeric_reply(ERR_CHANOPRIVSNEEDED, info);
    if (!channel->mode_str_has_enough_params(params[1], params.size() - 2))
        return send_numeric_reply(ERR_NEEDMOREPARAMS, info);
    if (params[1][0] != '+' && params[1][0] != '-')
    {
        info["char"] = params[1][0];
        return send_numeric_reply(ERR_UNKNOWNMODE, info);
    }
    change_mode = channel->parse_mode(*this, params[1], params);
    change_mode_str = channel->change_mode(change_mode);
    if (!change_mode_str.empty())
        channel->notify(this->full_nickname, info["command"], change_mode_str);
}


// ============================
//            PING
// ============================

void Client::ping(std::vector<std::string> const &params)
{
    std::string msg;
    
    msg = app.create_message(app.server_name, "PONG", app.server_name + ' ' + (params.empty() ? "" : params[0]));
    send_message(msg);
}

