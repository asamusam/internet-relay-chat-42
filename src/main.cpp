#include "App.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    App app("127.0.0.1", "password");
    
    Client *first_client = new Client;
    Client *second_client = new Client;

    first_client->uuid = 1;
    first_client->fd = 3;
    first_client->nickname = "bant";
    first_client->username = "bantik";
    first_client->has_valid_pwd = true;
    first_client->is_registered = true;

    second_client->uuid = 2;
    second_client->fd = 4;
    second_client->nickname = "";
    second_client->username = "";
    second_client->has_valid_pwd = true;
    second_client->is_registered = false;

    app.add_client(first_client);
    app.add_client(second_client);

    std::string msg;
    if (argc == 2)
        msg = argv[1];
    else
        msg = "PASS wrong";

    std::string reply;
    int res = app.parse_message(*second_client, msg, reply);
    if (res == 0)
    {
        std::cout << "Message parsed successfully!" << std::endl;
        std::cout << "Command executed." << std::endl;
    }
    else if (res > 0)
    {
        std::cout << "Formed error reply\n" << reply << std::endl;
    }
    else
    {
        std::cout << "Ignoring this message." << std::endl;
    }
    delete first_client;
    return 0;
}
