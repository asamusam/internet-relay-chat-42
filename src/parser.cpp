#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include "ircserv.hpp"

/*
The server to which a client is connected is required to parse the
complete message, returning any appropriate errors. If the server
encounters a fatal error while parsing a message, an error must be
sent back to the client and the parsing terminated.  A fatal error
may be considered to be incorrect command, a destination which is
otherwise unknown to the server (server, nick or channel names fit
this category), not enough parameters or incorrect privileges.

If a full set of parameters is presented, then each must be checked
for validity and appropriate responses sent back to the client.  In
the case of messages which use parameter lists using the comma as an
item separator, a reply must be sent for each item.
*/

/*
"Clients should not use prefix when sending a message from
themselves; if they use a prefix, the only valid prefix is the
registered nickname associated with the client.  If the source
identified by the prefix cannot be found from the server's internal
database, or if the source is registered from a different link than
from which the message arrived, the server must ignore the message
silently".
*/

/*
The numeric reply must be sent as one
message consisting of the sender prefix, the three digit numeric, and
the target of the reply.
*/

/*
A numeric reply is not allowed to originate
from a client; any such messages received by a server are silently
dropped.
*/


client const *findClientByName(app const &data, std::string const &name)
{
    for (std::vector<client *>::const_iterator i = data.clients.begin(); i < data.clients.end(); i++)
    {
        if ((*i)->registered && (*i)->nickname == name)
            return *i;
    }
    return NULL;
}

// check if the nickname in the prefix is registered and belong to the source
bool isValidPrefix(app const &data, client const &user, std::string const &prefix)
{
    client const *clientFound;
    
    if (user.nickname.empty())
        return false;

    clientFound = findClientByName(data, prefix);
    if (!clientFound || clientFound->uuid != user.uuid)
        return false;

    return true;       
}


// check if the command is only letters and belong to a list of approved commands
// check if the params are valid for this command (run a corresponding function from a list)
int checkCommand(std::string const &command);


// -1 do nothing
// 0 valid message, check the buffer
// > 0 error reply

// any command sent by the client until both username and nickname are set is invalid and silently ignored
int parseMessage(app const &data, client const &user, std::string const &msg, message &resMsg)
{
    std::istringstream msgStream(msg);
    std::string prefix;
    std::string command;
    std::string params;

    (void)resMsg;

    if (*msg.begin() == ':')
    {
        std::getline(msgStream, prefix, ' ');
        prefix.erase(prefix.begin());

        if (isValidPrefix(data, user, prefix) == false)
        {
            std::cout << "Invalid prefix: " << prefix << std::endl;
            return -1;
        }
    }

    msgStream >> std::ws;
    std::getline(msgStream, command, ' ');
    
    msgStream >> std::ws;
    std::getline(msgStream, params, ' ');

    std::cout << "prefix: " << prefix << "\ncommand: " << command << "\nparams: " << params << std::endl;

    return 0;
}
