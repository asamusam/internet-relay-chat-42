#include "App.hpp"
#include "Channel.hpp"
#include "Client.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include <ctime>

// ============================
//   Constructor & Destructor
// ============================

App::App(std::string const &name, std::string const &password) : server_password(password), server_name(name)
{
    std::time_t result = std::time(NULL);
    
    this->server_version = "1.0";
    this->network_name = "42 London";
    this->created_at = std::asctime(std::localtime(&result));
    commands.push_back((Command){"PASS",    &Client::pass});
    commands.push_back((Command){"NICK",    &Client::nick});
    commands.push_back((Command){"USER",    &Client::user});
    commands.push_back((Command){"JOIN",    &Client::join});
    commands.push_back((Command){"PRIVMSG", &Client::privmsg});
    commands.push_back((Command){"KICK",    &Client::kick});
    commands.push_back((Command){"INVITE",  &Client::invite});
    commands.push_back((Command){"TOPIC",   &Client::topic});
    commands.push_back((Command){"MODE",    &Client::mode});
    commands.push_back((Command){"PING",    &Client::ping});
    commands.push_back((Command){"WHOIS",   NULL});
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
    for (std::map<uint32, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
        delete it->second;
}

void App::free_channels(void)
{
    for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++)
        delete it->second;
}


// ============================
//          Checkers
// ============================

bool App::is_correct_pwd(std::string const &password) const
{
    return password == server_password;
}


// ============================
//          Clients
// ============================

void App::add_client(Client *new_client)
{
    clients[new_client->get_uuid()] = new_client;
}

void App::remove_client(int uuid)
{
    std::map<uint32, Client *>::iterator it;

    it = clients.find(uuid);
    if (it == clients.end())
        return ;
    delete it->second;
    clients.erase(uuid);
}

Client *App::get_client(uint32 uuid) const
{
	std::map<uint32, Client *>::const_iterator it;		
	
	it = clients.find(uuid);
	
	if (it == clients.end())
		return (NULL);
	return (it->second);
}

Client *App::find_client_by_nick(std::string const &nick) const
{
    for (std::map<uint32, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->get_nickname() == nick)
            return i->second;
    }
    return NULL;
}

Client *App::find_client_by_fd(int fd) const
{
    for (std::map<uint32, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->get_fd() == fd)
            return i->second;
    }
    return NULL;
}

// ============================
//          Channels
// ============================

Channel *App::create_channel(std::string const &nick, std::string const &channel_name)
{
    Channel *channel;

    channel = new Channel(*this, nick, channel_name);

    return channel;
}

void App::add_channel(Channel *channel)
{
    channels[channel->name] = channel;
}

void App::remove_channel(std::string const &channel_name)
{
    std::map<std::string, Channel *>::iterator it;

    it = channels.find(channel_name);
    if (it == channels.end())
        return ;
    delete it->second;
    channels.erase(channel_name);
}

Channel *App::find_channel_by_name(std::string const &channel_name) const
{
    std::map<std::string, Channel *>::const_iterator it;

    it = channels.find(channel_name);
    if (it == channels.end())
        return NULL;
    return it->second;
}


// ============================
//       Helper functions
// ============================

static void skip_space(std::istringstream &msg_stream)
{
    while (msg_stream.peek() == ' ')
        msg_stream.ignore(1);
}

std::string App::create_message(std::string const &prefix, std::string const &cmd, std::string const &msg)
{
    return ':' + prefix + ' ' + cmd + ' ' + msg;
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
            if (i->cmd_func)
                return (user.*(i->cmd_func))(msg.params);
            else
                return ;
        }
    }
    info["client"] = user.get_full_nickname();
    info["command"] = msg.command;
    user.send_numeric_reply(ERR_UNKNOWNCOMMAND, info);
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

    if (!msg.params.empty())
        msg.params.clear();

    if (msg_stream.peek() == ':')
    {
        msg_stream.ignore(1);
        std::getline(msg_stream, msg.prefix, ' ');
        if (!user.is_registered_client() || user.get_nickname() != msg.prefix)
            return -1;
        skip_space(msg_stream);
    }

    std::getline(msg_stream, msg.command, ' ');
    skip_space(msg_stream);

    while (!msg_stream.eof())
    {
        if (msg_stream.peek() == ':')
        {
            // msg_stream.ignore(1);
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
