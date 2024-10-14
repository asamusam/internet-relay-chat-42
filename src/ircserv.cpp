#include "ircserv.hpp"
#include "App.hpp"

static void printMsg(message const *msg)
{
    std::cout << "Prefix: " << msg->prefix << '\n'
              << "Command: " << msg->command << '\n'
              << "Params: ";
    for (std::vector<std::string>::const_iterator i = msg->params.begin(); i < msg->params.end(); i++)
    {
        std::cout << *i << " ";
    }
    std::cout << std::endl;
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    App app("Dream Team");

    client *firstClient = new client;
    message *resMsg = new message;
    
    firstClient->uuid = 1;
    firstClient->nickname = "bant";
    firstClient->fd = 3;
    firstClient->registered = true;
    firstClient->password = false;

    app.addClient(firstClient);

    std::string msg(":bant       NICK bantozavr");

    int res = app.parseMessage(msg, *firstClient, *resMsg);
    if (res == 0)
    {
        std::cout << "Message parsed successfully!" << std::endl;
        printMsg(resMsg);
    }
    else if (res > 0)
    {
        std::cout << "There was an error! Formed an error mesage to send back to the client." << std::endl;
        printMsg(resMsg);
    }
    else
    {
        std::cout << "Something went wrong during parsing, I'm ignoring it." << std::endl;
    }
    
    return 0;
}
