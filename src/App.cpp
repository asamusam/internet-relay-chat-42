#include "App.hpp"

#include <sstream>
#include <algorithm>

enum ErrorCodes {
    ERR_UNKNOWNCOMMAND = 421,
    ERR_NEEDMOREPARAMS = 461,
    ERR_ALREADYREGISTRED = 462
};

static std::string intToString(int number)
{
    std::stringstream ss;
    
    ss << number;
    
    return ss.str();
}

App::App(std::string name) : serverName(name)
{
    Command cmd1 = {"PASS", &App::checkPass, NULL};
    Command cmd2 = {"NICK", &App::checkNick, NULL};
    Command cmd3 = {"USER", &App::checkUser, NULL};
    Command cmd4 = {"JOIN", &App::checkJoin, NULL};
    Command cmd5 = {"PRIVMSG", &App::checkPrivmsg, NULL};
    Command cmd6 = {"KICK", &App::checkKick, NULL};
    Command cmd7 = {"INVITE", &App::checkInvite, NULL};
    Command cmd8 = {"TOPIC", &App::checkTopic, NULL};
    Command cmd9 = {"MODE", &App::checkMode, NULL};

    commands.push_back(cmd1);
    commands.push_back(cmd2);
    commands.push_back(cmd3);
    commands.push_back(cmd4);
    commands.push_back(cmd5);
    commands.push_back(cmd6);
    commands.push_back(cmd7);
    commands.push_back(cmd8);
    commands.push_back(cmd9);

    errorMessages[ERR_UNKNOWNCOMMAND] = "<command> :Unknown command";
    errorMessages[ERR_NEEDMOREPARAMS] = "<command> :Not enough parameters";
    errorMessages[ERR_ALREADYREGISTRED] = ":You may not reregister";
}

void App::addClient(client *newClient)
{
    clients.push_back(newClient);
}

int App::checkCommand(client const &user, std::string const &cmd, std::string const &params) const
{
    for (std::vector<Command>::const_iterator i = commands.begin(); i < commands.end(); i++)
    {
        if (i->name == cmd)
        {
            return (this->*(i->checkCmd))(user, cmd, params);
        }
    }
    return ERR_UNKNOWNCOMMAND;
}


int App::checkPass(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}


int App::checkNick(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkUser(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkJoin(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkPrivmsg(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkKick(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkInvite(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkTopic(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::checkMode(client const &user, std::string const &cmd, std::string const &params) const
{
   return 0; 
}

int App::parseMessage(std::string const &msg, client const &user, message &resMsg) const
{
    std::istringstream msgStream(msg);

    std::string prefix;
    std::string cmd;
    std::string params;

    (void)resMsg;

    if (*msg.begin() == ':')
    {
        std::getline(msgStream, prefix, ' ');
        prefix.erase(prefix.begin());

        if (isValidPrefix(user, prefix) == false)
        {
            std::cout << "Invalid prefix: " << prefix << std::endl;
            return -1;
        }
    }

    msgStream >> std::ws;
    std::getline(msgStream, cmd, ' ');
    
    msgStream >> std::ws;
    std::getline(msgStream, params, ' ');

    int res = checkCommand(user, cmd, params);
    if (res == -1)
        return -1;
    else if (res == 0)
    {
        resMsg.prefix = user.nickname;
        resMsg.command = cmd;
        resMsg.params.push_back(params);
        return 0;
    }
    else
    {
        resMsg.prefix = this->serverName;
        resMsg.command = intToString(res);
        resMsg.params.push_back(errorMessages.find(res)->second);
        return res;
    }
}


bool App::isValidPrefix(client const &user, std::string const &prefix) const
{
    int clientIdFound;
    
    if (user.nickname.empty())
        return false;

    clientIdFound = getClientIdByName(prefix);
    if (clientIdFound == -1 || clientIdFound != user.uuid)
        return false;

    return true;
}


int App::getClientIdByName(std::string const &name) const
{
    for (std::vector<client *>::const_iterator i = clients.begin(); i < clients.end(); i++)
    {
        if ((*i)->registered && (*i)->nickname == name)
            return (*i)->uuid;
    }
    return -1;
}