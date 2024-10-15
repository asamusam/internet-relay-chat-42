#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>

struct Channel
{
    std::string name;
    std::string topic;
    std::vector<int> client_ids;
};

#endif // CHANNEL_HPP