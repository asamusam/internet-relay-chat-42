#include "IRCReply.hpp"

#include <sstream>

std::pair<IRCReplyCodeEnum, std::string> reply_data[] = {
	std::make_pair(ERR_UNKNOWNCOMMAND,    "<client> <command> :Unknown command"),
	std::make_pair(ERR_NEEDMOREPARAMS,    "<client> <command> :Not enough parameters"),
	std::make_pair(ERR_ALREADYREGISTERED, "<client> :You may not reregister"),
	std::make_pair(ERR_PASSWDMISMATCH,    "<client> :Password incorrect"),
	std::make_pair(ERR_NICKNAMEINUSE,     "<client> <nick> :Nickname is already in use"),
	std::make_pair(ERR_NONICKNAMEGIVEN,   "<client> :No nickname given"),
	std::make_pair(ERR_ERRONEUSNICKNAME,  "<client> <nick> :Erroneous nickname"),
	std::make_pair(ERR_NORECIPIENT,       "<client> :No recipient given (<command>)"),
	std::make_pair(ERR_NOTEXTTOSEND,      "<client> :No text to send"),
	std::make_pair(ERR_CANNOTSENDTOCHAN,  "<client> <channel> :Cannot send to channel"),
	std::make_pair(ERR_TOOMANYTARGETS,    "<client> <target> :Duplicate recipients. No message delivered"),
	std::make_pair(ERR_NOSUCHNICK,        "<client> <nick> :No such nick/channel"),
	std::make_pair(ERR_NOSUCHCHANNEL,     "<client> <channel> :No such channel"),
	std::make_pair(ERR_TOOMANYCHANNELS,   "<client> <channel> :You have joined too many channels"),
	std::make_pair(ERR_INVITEONLYCHAN,    "<client> <channel> :Cannot join channel (invite only)"),
	std::make_pair(ERR_BANNEDFROMCHAN,    "<client> <channel> :Cannot join channel (banned)"),
	std::make_pair(ERR_CHANNELISFULL,     "<client> <channel> :Cannot join channel (channel is full)"),
	std::make_pair(ERR_BADCHANNELKEY,     "<client> <channel> :Cannot join channel (incorrect channel key)"),
	std::make_pair(ERR_BADCHANMASK,       "<channel> :Bad Channel Mask"),
	std::make_pair(ERR_USERONCHANNEL,     "<client> <user> <channel> :is already on channel"),
	std::make_pair(ERR_USERNOTINCHANNEL,  "<client> <user> <channel> :They are not on that channel"),
	std::make_pair(ERR_NOTONCHANNEL,      "<client> <channel> :You're not on that channel"),
	std::make_pair(ERR_CHANOPRIVSNEEDED,  "<client> <channel> :You're not channel operator"),
	std::make_pair(ERR_KEYSET,            "<client> <channel> :Channel key already set"),
	std::make_pair(ERR_UNKNOWNMODE,       "<client> <char> :is unknown mode char to me for <channel>"),
	std::make_pair(ERR_USERSDONTMATCH,    "<client> :Cannot change mode for other users"),
	std::make_pair(RPL_TOPIC,             "<client> <channel> <topic>"),
	std::make_pair(RPL_NAMREPLY,          "<client> <symbol> <channel> :<nicks>"),
	std::make_pair(RPL_INVITING,          "<client> <nick> <channel>"),
	std::make_pair(RPL_CHANNELMODEIS,     "<client> <channel> <mode> <mode params>"),
	std::make_pair(RPL_NOTOPIC,           "<client> <channel> :No topic is set"),
	std::make_pair(RPL_WELCOME,           "<nick> :*** Welcome to <network>, <nick>! ***"),
	std::make_pair(RPL_YOURHOST,          "<client> :Your host is <servername>, running version <version>"),
	std::make_pair(RPL_CREATED,           "<client> :This server was created <datetime>")
};

std::map<IRCReplyCodeEnum, std::string> IRCReply::reply_messages(reply_data, reply_data + sizeof reply_data / sizeof reply_data[0]);

const std::string& IRCReply::get_reply_message(IRCReplyCodeEnum iec)
{
	return (reply_messages[iec]);
}

std::string IRCReply::code_to_string(IRCReplyCodeEnum code)
{
	std::stringstream ss;

	ss << code;
	
	if (code < 10)
		return "00" + ss.str();
	
	return ss.str();
}
