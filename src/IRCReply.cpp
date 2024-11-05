#include "IRCReply.hpp"

std::pair<IRCReplyCodeEnum, std::string> reply_data[] = {
    std::make_pair(ERR_UNKNOWNCOMMAND,   "<command> :Unknown command"),
    std::make_pair(ERR_NEEDMOREPARAMS,   "<command> :Not enough parameters"),
    std::make_pair(ERR_ALREADYREGISTRED, ":You may not reregister"),
    std::make_pair(ERR_PASSWDMISMATCH,   ":Password incorrect"),
    std::make_pair(ERR_NICKNAMEINUSE,    "<nick> :Nickname is already in use"),
    std::make_pair(ERR_NONICKNAMEGIVEN,  ":No nickname given"),
    std::make_pair(ERR_ERRONEUSNICKNAME, "<nick> :Erroneus nickname"),
    std::make_pair(ERR_NORECIPIENT,      ":No recipient given (<command>)"),
    std::make_pair(ERR_NOTEXTTOSEND,     ":No text to send"),
    std::make_pair(ERR_CANNOTSENDTOCHAN, "<channel> :Cannot send to channel"),
    std::make_pair(ERR_TOOMANYTARGETS,   "<target> :Duplicate recipients. No message delivered"),
    std::make_pair(ERR_NOSUCHNICK,       "<nick> :No such nick/channel"),
    std::make_pair(ERR_NOSUCHCHANNEL,    "<channel> :No such channel"),
    std::make_pair(ERR_TOOMANYCHANNELS,  "<channel> :You have joined too many channels"),
    std::make_pair(ERR_INVITEONLYCHAN,   "<channel> :Cannot join channel (invite only)"),
    std::make_pair(ERR_BANNEDFROMCHAN,   "<channel> :Cannot join channel (banned)"),
    std::make_pair(ERR_CHANNELISFULL,    "<channel> :Cannot join channel (channel is full)"),
    std::make_pair(ERR_BADCHANNELKEY,    "<channel> :Cannot join channel (incorrect channel key)"),
    std::make_pair(ERR_BADCHANMASK,      "<channel> :Bad Channel Mask"),
    std::make_pair(ERR_USERONCHANNEL,    "<user> <channel> :is already on channel"),
    std::make_pair(ERR_USERNOTINCHANNEL, "<user> <channel> :They are not on that channel"),
    std::make_pair(ERR_NOTONCHANNEL,     "<channel> :You're not on that channel"),
    std::make_pair(ERR_CHANOPRIVSNEEDED, "<channel> :You're not channel operator"),
    std::make_pair(ERR_KEYSET,           "<channel> :Channel key already set"),
    std::make_pair(ERR_UNKNOWNMODE,      "<char> :is unknown mode char to me for <channel>"),
    std::make_pair(ERR_NOPRIVILEGES,     ":Permission Denied- You're not an IRC operator"),
    std::make_pair(ERR_USERSDONTMATCH,   ":Cannot change mode for other users"),
    std::make_pair(RPL_TOPIC,            "<channel> : <topic>"),
    std::make_pair(RPL_NAMREPLY,         "<client> <symbol> <channel> :<nicks>"),
    std::make_pair(RPL_INVITING,         "<channel> <nick>"),
    std::make_pair(RPL_CHANNELMODEIS,    "<channel> <mode> <mode params>"),
    std::make_pair(RPL_NOTOPIC,          "<channel> :No topic is set"),
    std::make_pair(RPL_WELCOME,          "<nick> :Welcome to <network>, <nick>")
};

std::map<IRCReplyCodeEnum, std::string> IRCReply::reply_messages(reply_data, reply_data + sizeof reply_data / sizeof reply_data[0]);

const std::string& IRCReply::get_reply_message(IRCReplyCodeEnum iec)
{
	return (reply_messages[iec]);
}