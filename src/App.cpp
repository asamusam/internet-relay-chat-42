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
    if (!user.has_valid_pwd)
        return ERR_PASSWDMISMATCH;
    if (params.empty())
        return ERR_NONICKNAMEGIVEN;
    if (!nick_is_valid(params[0]))
        return ERR_ERRONEUSNICKNAME;
    if (find_client_by_nick(params[0]))
        return ERR_NICKNAMEINUSE;
    user.nickname = params[0];
    if (!user.is_registered && !user.username.empty())
        user.is_registered = true;
    //std::cout << "New nickname: " << user.nickname 
    //          << (user.is_registered ? "\nRegistration complete." : "") << std::endl;
    return 0; 
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
int App::user(Client &user, std::vector<std::string> const &params)
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;
    if (!user.has_valid_pwd)
        return ERR_PASSWDMISMATCH;
    if (params.empty())
        return ERR_NEEDMOREPARAMS;
    if (params[0].find(' ') != params[0].npos)
        return -1;
    if (params[0].length() > 12)
        user.username = params[0].substr(0, 12);
    else
        user.username = params[0];
    if (!user.nickname.empty())
        user.is_registered = true;
    //std::cout << "New username: " << user.username 
    //          << (user.is_registered ? "\nRegistration complete." : "") << std::endl;
    return 0;
}

int App::join(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;
    return 0; 
}

/*
Parameters: <receiver>{,<receiver>} <text to be sent>
<receiver> can be a nickname or a channel
*/
int App::privmsg(Client &user, std::vector<std::string> const &params)
{
    if (!user.is_registered)
        return -1;

    // split receivers
    // try to send message to every one of them
    // send replies accordingly for each
    
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

std::string App::numeric_reply(int err, std::string const &var) const
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
    return reply;
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
    reply = numeric_reply(ERR_UNKNOWNCOMMAND, msg.command);
    send_message(user, reply);
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
        std::getline(msg_stream, msg.prefix, ' ');
        if (!user.is_registered || user.nickname != msg.prefix)
            return -1;
    }

    skip_space(msg_stream);
    std::getline(msg_stream, msg.command, ' ');

    while (!msg_stream.eof())
    {
        skip_space(msg_stream);
        if (msg_stream.peek() == ':')
        {
            msg_stream.ignore(1);
            std::getline(msg_stream, param);
        }
        else
            std::getline(msg_stream, param, ' ');
        msg.params.push_back(param);
    }
    return 0;
}
