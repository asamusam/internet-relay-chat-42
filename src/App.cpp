#include "App.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

static std::string int_to_string(int number)
{
    std::stringstream ss;

    ss << number;
    
    return ss.str();
}

App::App(std::string const &name, std::string const &password) : server_name(name), server_password(password)
{
    commands.push_back((Command){"PASS", &App::check_pass, NULL});
    commands.push_back((Command){"NICK", &App::check_nick, NULL});
    commands.push_back((Command){"USER", &App::check_user, NULL});
    commands.push_back((Command){"JOIN", &App::check_join, NULL});
    commands.push_back((Command){"PRIVMSG", &App::check_privmsg, NULL});
    commands.push_back((Command){"KICK", &App::check_kick, NULL});
    commands.push_back((Command){"INVITE", &App::check_invite, NULL});
    commands.push_back((Command){"TOPIC", &App::check_topic, NULL});
    commands.push_back((Command){"MODE", &App::check_mode, NULL});

    error_messages[ERR_UNKNOWNCOMMAND] = "<command> :Unknown command";
    error_messages[ERR_NEEDMOREPARAMS] = "<command> :Not enough parameters";
    error_messages[ERR_ALREADYREGISTRED] = ":You may not reregister";
}

void App::add_client(Client *new_client)
{
    clients[new_client->uuid] = new_client;
}

// any command sent by the client until both username and nickname are set is invalid and silently ignored
int App::check_command(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    for (std::vector<Command>::const_iterator i = commands.begin(); i < commands.end(); i++)
    {
        if (i->name == cmd)
        {
            return (this->*(i->check_cmd))(user, cmd, params);
        }
    }
    return ERR_UNKNOWNCOMMAND;
}

int App::check_pass(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;
    
    return 0;
}

int App::check_nick(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;

    return 0; 
}

int App::check_user(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    if (user.is_registered)
        return ERR_ALREADYREGISTRED;

    return 0; 
}

int App::check_join(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    return 0; 
}

int App::check_privmsg(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    return 0; 
}

int App::check_kick(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    return 0; 
}

int App::check_invite(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    return 0; 
}

int App::check_topic(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    return 0; 
}

int App::check_mode(Client const &user, std::string const &cmd, std::vector<std::string> const &params) const
{
    return 0; 
}

// -1 do nothing
// 0 valid message, check the buffer
// > 0 error reply
int App::parse_message(std::string const &msg, Client const &user, Message &res_msg) const
{
    std::istringstream msg_stream(msg);
    std::string prefix;
    std::string cmd;
    std::string param;
    std::vector<std::string> params;
    std::string trailing;
    int res_check;

    // THE PROBLEM: >> operator treats any whitespace as delimiter, not only ' '
    if (msg[0] == ':')
    {
        msg_stream >> prefix;
        prefix.erase(prefix.begin());

        if (!is_valid_prefix(user, prefix))
        {
            return -1;
        }
    }

    msg_stream >> cmd;
    while (msg_stream >> param)
    {
        if (param[0] == ':')
        {
            std::getline(msg_stream, trailing);
            params.push_back(param.substr(1) + trailing); // WRONG
        }
        params.push_back(param);
    }

    res_check = check_command(user, cmd, params);
    if (res_check == -1)
        return -1;
    else if (res_check == 0)
    {
        res_msg.prefix = user.nickname;
        res_msg.command = cmd;
        res_msg.params = params;
        return 0;
    }
    else
    {
        res_msg.prefix = this->server_name;
        res_msg.command = int_to_string(res_check);
        res_msg.params.push_back(error_messages.find(res_check)->second);
        return res_check;
    }
}

// check if the nickname in the prefix is registered and belong to the source
bool App::is_valid_prefix(Client const &user, std::string const &prefix) const
{
    int client_id_found;

    if (user.nickname.empty())
        return false;

    client_id_found = get_client_id_by_name(prefix);
    if (client_id_found == -1 || client_id_found != user.uuid)
        return false;

    return true;
}

int App::get_client_id_by_name(std::string const &name) const
{
    for (std::map<int, Client *>::const_iterator i = clients.begin(); i != clients.end(); i++)
    {
        if (i->second->is_registered && i->second->nickname == name)
            return i->second->uuid;
    }
    return -1;
}
