#include "App.hpp"
#include <iostream>

// static void printMsg(Message const &msg)
// {
//     std::cout << "Prefix: " << msg.prefix << '\n'
//               << "Command: " << msg.command << '\n'
//               << "Params: ";
//     for (std::vector<std::string>::const_iterator i = msg.params.begin(); i < msg.params.end(); i++)
//     {
//         std::cout << *i << ", ";
//     }
//     std::cout << std::endl;
// }

int main(int argc, char **argv)
{
    App app("127.0.0.1", "password");
    
    Client *first_client = new Client;
    Client *second_client = new Client;
    Channel *channel = new Channel("home");

    first_client->uuid = 1;
    first_client->fd = 3;
    first_client->nickname = "bant";
    first_client->username = "";
    first_client->has_valid_pwd = true;
    first_client->is_registered = true;
    first_client->num_channels = 0;

    second_client->uuid = 2;
    second_client->fd = 4;
    second_client->nickname = "qoops";
    second_client->username = "qoops";
    second_client->has_valid_pwd = true;
    second_client->is_registered = true;
    first_client->num_channels = 0;

    app.add_client(first_client);
    app.add_client(second_client);
    channel->add_client(first_client->nickname);
    //channel->add_operator(first_client->nickname);
    // channel->add_client(second_client->nickname);
    channel->set_topic("funfunfun");
    app.add_channel(channel);

    std::string msg_string;
    if (argc == 2)
        msg_string = argv[1];
    else
        msg_string = "PASS wrong";

    int res;
    Message msg;
    res = app.parse_message(*first_client, msg_string, msg);
    //printMsg(msg);
    if (res == -1)
        std::cout << "Ignore" << std::endl;
    else
    {
        app.execute_message(*first_client, msg);
    }
    // delete first_client;
    // delete second_client; 
    // delete channel;
    return 0;
}
