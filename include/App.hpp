#ifndef APP_HPP
#define APP_HPP

#include "ircserv.hpp"
#include <map>

class App;

typedef struct sCommand
{
    std::string name;
    int (App::*checkCmd)(client const &user, std::string const &cmd, std::string const &params) const;
    int (*executeCmd)(message const &msg);
} Command;

class App
{
    private:
        std::string serverName;
        std::vector<Command> commands;
        std::map<int, std::string> errorMessages;
        std::vector<client *> clients;
        std::vector<channel *> channels;
    public:
        App(std::string name);

        void addClient(client *newClient);

        int parseMessage(std::string const &msg, client const &user, message &resMsg) const;
        bool isValidPrefix(client const &user, std::string const &prefix) const;
        
        int checkCommand(client const &user, std::string const &cmd, std::string const &params) const;
        int checkPass(client const &user, std::string const &cmd, std::string const &params) const;
        int checkNick(client const &user, std::string const &cmd, std::string const &params) const;
        int checkUser(client const &user, std::string const &cmd, std::string const &params) const;
        int checkJoin(client const &user, std::string const &cmd, std::string const &params) const;
        int checkPrivmsg(client const &user, std::string const &cmd, std::string const &params) const;
        int checkKick(client const &user, std::string const &cmd, std::string const &params) const;
        int checkInvite(client const &user, std::string const &cmd, std::string const &params) const;
        int checkTopic(client const &user, std::string const &cmd, std::string const &params) const;
        int checkMode(client const &user, std::string const &cmd, std::string const &params) const;

        int getClientIdByName(std::string const &name) const;
};

#endif // APP_HPP