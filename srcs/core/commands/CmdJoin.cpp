#include "Server.hpp"

void Server::handleJoin(int fd, std::istringstream& iss)
{
	std::string chanName, key;
	iss >> chanName >> key;

	if (!Channel::isValidName(chanName))
	{
		sendNumeric(fd, 476, "*", chanName + " :Invalid channel name");
		return;
	}

	Channel& channel = getOrCreateChannel(chanName, fd);
	if (!channel.hasClient(fd) && !tryJoinChannel(fd, channel, key))
		return;
	//if (channel.getClientCount() >= 1 && !tryJoinChannel(fd, channel, key))
		//return;
	
	announceJoin(channel, fd);
	sendChannelTopic(channel, fd);
	sendNamesReply(channel, fd);
}
