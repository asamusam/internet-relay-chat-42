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
}

void App::add_client(Client *new_client)
{
    clients[new_client->uuid] = new_client;
}

/*
If the message contains multiple parameters for PASS command,
only the first one is taken into consideration and the rest are ignored.

If the user sends an additional PASS command with incorrect password
after already setting nickname or username, the server replies with ERR_PASSWDMISMATCH,
and future attempts to complete the registration with NICK/USER commands lead
to the same result until the correct password is set.
*/
int App::pass(Client &user, std::vector<std::string> const &params)
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;
    if (params.empty())
        return ERR_NEEDMOREPARAMS;
    if (params[0] != this->server_password)
    {
        if (user.has_valid_pwd)
            user.has_valid_pwd = false;
        return ERR_PASSWDMISMATCH;
    }
    user.has_valid_pwd = true;
    return 0;
}

/*
If the nickname is already used by other client,
the server just sends back ERR_NICKNAMEINUSE reply.
*/
int App::nick(Client &user, std::vector<std::string> const &params)
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;
    if (!user.has_valid_pwd)
        return ERR_PASSWDMISMATCH;
    if (params.empty())
        return ERR_NONICKNAMEGIVEN;
    if (!nick_is_valid(params[0]))
        return ERR_ERRONEUSNICKNAME;
    if (find_client_by_nick(params[0]))
        return ERR_NICKNAMEINUSE;
    user.nickname = params[0];
    return 0; 
}

/*
Nickname has a maximum length of nine (9) characters.
<nick>>: <letter> { <letter> | <number> | <special> }
<letter>     ::= 'a' ... 'z' | 'A' ... 'Z'
<number>     ::= '0' ... '9'
<special>    ::= '-' | '[' | ']' | '\' | '`' | '^' | '{' | '}'
*/
bool App::nick_is_valid(std::string const &nick) const
{
    size_t nick_len = nick.length();
    std::string special("-[]\\`^{}");

    if (nick_len > 9)
        return false;
    if (!std::isalpha(nick[0]))
        return false;
    for (size_t i = 0; i < nick_len; i++)
    {
        if (!std::isalnum(nick[i]) && special.find(nick[i]) == special.npos)
            return false;
    }
    return true;
}

// TODO: MAYBE NOT THE BEST IDEA TO STORE CLIENTS WITH IDS AS KEYS: NICK IS USED MORE OFTEN TO FIND A CLIENT
Client *App::find_client_by_nick(std::string const &nick) const
{
    for (std::map<int, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->nickname == nick)
            return i->second;
    }
    return NULL;
}


int App::user(Client &user, std::vector<std::string> const &params)
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;
    if (!user.has_valid_pwd)
        return ERR_PASSWDMISMATCH;
    return 0; 
}

int App::join(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

int App::privmsg(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

int App::kick(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

int App::invite(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

int App::topic(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

int App::mode(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

int App::run_cmd(Client &user, Message const &msg)
{
    for (std::vector<Command>::const_iterator i = commands.begin(); i < commands.end(); i++)
    {
        if (i->name == msg.command)
            return (this->*(i->cmd_func))(user, msg.params);
    }
    return ERR_UNKNOWNCOMMAND;
}

// -1 do nothing
// 0 valid message, check the buffer
// > 0 error reply
int App::parse_message(Client &user, std::string const &msg_string, std::string &reply)
{
    std::istringstream msg_stream(msg_string);
    Message msg;
    std::string param;
    std::string trailing;
    int res;

    // THE PROBLEM: >> operator treats any whitespace as delimiter, not only ' '
    if (msg_string[0] == ':')
    {
        msg_stream >> msg.prefix;
        msg.prefix.erase(msg.prefix.begin());

        if (!user.is_registered || user.nickname != msg.prefix)
        {
            return -1;
        }
    }

    msg_stream >> msg.command;
    while (msg_stream >> param)
    {
        if (param[0] == ':')
        {
            std::getline(msg_stream, trailing);
            msg.params.push_back(param.substr(1) + trailing); // WRONG, COULD BE MULTIPLE SPACES
        }
        msg.params.push_back(param);
    }

    res = run_cmd(user, msg);
    if (res > 0)
    {
        reply = ':' + this->server_name + ' ' + int_to_string(res) + ' ' + error_messages[res];
    }
    return res;
}
