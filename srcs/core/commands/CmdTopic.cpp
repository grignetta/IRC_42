#include "Server.hpp"

bool Server::verifyTopicParams(int fd, const std::string& chanName)
{
	if (chanName.empty())
	{
		sendNumeric(fd, 461, "TOPIC", ":Not enough parameters"); // ERR_NEEDMOREPARAMS
		return false;
	}
	return true;
}

bool Server::permitTopicChange(int fd, Channel& channel, const std::string& chanName)
{
	if (!channel.isOperator(fd) && channel.isTopicRestricted())
	{
		sendNumeric(fd, 482, chanName, "You're not channel operator"); // ERR_CHANOPRIVSNEEDED
		return false;
	}
	return true;
}

std::string Server::updateTopic(std::istringstream& iss)
{
	std::streampos before = iss.tellg();
	bool haveBookmark = (before != std::streampos(-1));
	try
	{ 
		return parseTrailing(iss);
	}
	catch (const std::exception& e)
	{
		iss.clear();
		if (haveBookmark) {
			iss.seekg(before);
			if (!iss) iss.clear();
		}
		throw; 
	}
}

void Server::broadcastTopic(Channel& channel, int setterFd, const std::string& newTopic)
{
	Client& who = _clients[setterFd];
	const std::string& host = who.getHostname();

	std::string announce = ":" + who.getNickname() + "!" + who.getUsername() +
					   "@" + host + " TOPIC " + channel.getName() + " :" + newTopic + "\r\n";//control!
	broadcastToChannel(channel, announce);
	//sendMsg(setterFd, wire);
}



void Server::handleTopic(int fd, std::istringstream& iss)
{
	std::string chanName;
	iss >> chanName;

	if (!verifyTopicParams(fd, chanName))
		return;
	Channel* channel = getChannel(chanName);
	if (!channel || !channel->hasClient(fd))
	{
		sendNumeric(fd, 442, chanName, ":You're not on that channel");
		return;
	}
	iss >> std::ws;
	if (iss.peek() == std::char_traits<char>::eof())
	{
		sendChannelTopic(*channel, fd); // you already have this
		return;
	}
	if (!permitTopicChange(fd, *channel, chanName))
		return;
	std::string newTopic;
	try
	{
		newTopic = updateTopic(iss);
	}
	catch (const std::exception& e)
	{
		sendNumeric(fd, 461, "TOPIC", e.what()); // 461: ERR_NEEDMOREPARAMS
		return;
	}
	channel->setTopic(newTopic);
	broadcastTopic(*channel, fd, newTopic);//not requested broadcasting to everyone?
}
