#include "Server.hpp"
#include "Channel.hpp"
#include <sstream>

void Server::handleKick(int fd, std::istringstream& iss)
{
	std::string chanName, targetNick, comment;
	iss >> chanName >> targetNick;

	if (!verifyKickParams(fd, chanName, targetNick))
		return;
	
	try
	{
		comment = parseTrailing(iss);
	}
	catch (const std::exception& e)
	{
		sendNumeric(fd, 461, "KICK", e.what()); // 461: ERR_NEEDMOREPARAMS
		return;
	}

	Channel* channel = getChannel(chanName);
	if (!channel)
	{
		sendNumeric(fd, 403, chanName, ":No such channel");
		return;
	}
	int targetFd = findClient(targetNick);
	if (!permitKick(fd, *channel, targetNick, targetFd))
		return;
	execKick(*channel, fd, targetNick, comment, targetFd);
}

bool Server::verifyKickParams(int fd, const std::string& chanName, const std::string& targetNick)
{
	if (chanName.empty() || targetNick.empty())
	{
		sendNumeric(fd, 461, "KICK", ":Not enough parameters");
		return false;
	}
	return true;
}

bool Server::permitKick(int fd, Channel& channel, const std::string& targetNick, int targetFd)
{
	if (!channel.hasClient(fd))
	{
		sendNumeric(fd, 442, channel.getName(), ":You're not on that channel");
		return false;
	}
	if (!channel.isOperator(fd))
	{
		sendNumeric(fd, 482, channel.getName(), ":You're not channel operator");
		return false;
	}
	if (targetFd == -1 || !channel.hasClient(targetFd))
	{
		sendNumeric(fd, 441, targetNick, channel.getName() + " :They aren't on that channel");
		return false;
	}
	if (channel.isOperator(targetFd) && channel.countOperators() <= 1)
		return false;
	return true;
}




void Server::execKick(Channel& channel, int kickerFd, const std::string& targetNick, const std::string& comment, int targetFd)//hope 5 are still fine?
{
	Client& kicker = _clients[kickerFd];
	std::string actualComment = comment.empty() ? kicker.getNickname() : comment;
	std::string msg = ":" + kicker.getNickname() + "!" + kicker.getUsername() +
		"@" + kicker.getHostname() + " KICK " + channel.getName() + " " + targetNick + " :" + actualComment + "\r\n";

	for (std::map<int, bool>::const_iterator it = channel.getMembers().begin();
		 it != channel.getMembers().end(); ++it)
	{
		sendMsg(it->first, msg);
	}
	channel.removeClient(targetFd);
}
