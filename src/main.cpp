#include "App.hpp"
#include <iostream>

static void print_msg(App::Message const *msg)
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

    App app("127.0.0.1", "password");

    App::Client *first_client = new App::Client;
    App::Message *res_msg = new App::Message;
    
    first_client->uuid = 1;
    first_client->nickname = "bant";
    first_client->fd = 3;
    first_client->registered = true;
    first_client->password = false;

    app.add_client(first_client);

    std::string msg("NICKnmnmn bantozavr");

    int res = app.parse_message(msg, *first_client, *res_msg);
    if (res == 0)
    {
        std::cout << "Message parsed successfully!" << std::endl;
        print_msg(res_msg);
    }
    else if (res > 0)
    {
        std::cout << "There was an error! Formed an error mesage to send back to the client." << std::endl;
        print_msg(res_msg);
    }
    else
    {
        std::cout << "Something went wrong during parsing, I'm ignoring it." << std::endl;
    }
    delete first_client;
    return 0;
}
