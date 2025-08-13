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

//Note that this message accepts a special argument ("0"), which is
//    a special request to leave all channels the user is currently a member
//    of.  The server will process this message as if the user had sent
//    a PART command (See Section 3.2.2) for each channel he is a member
//    of.  ??????????