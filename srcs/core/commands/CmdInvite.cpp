#include "Server.hpp"

void Server::handleInvite(int fd, std::istringstream& iss)
{
	std::string targetNick, chanName;
	iss >> targetNick >> chanName;

	if (targetNick.empty() || chanName.empty())// is this check enough?
	{
		sendNumeric(fd, 461, "INVITE", ":Not enough parameters");
		return;
	}

	int inviteeFd = findClient(targetNick);
	if (inviteeFd == -1)
	{
		sendNumeric(fd, 401, targetNick, ":No such nick");
		return;
	}

	if (!invitePerm(fd, inviteeFd, chanName))
		return;

	processInvite(fd, inviteeFd, targetNick, chanName);// sent even if a channel doesn't exist!
}

bool Server::invitePerm(int inviterFd, int inviteeFd, const std::string& chanName)
{
	std::map<std::string, Channel>::iterator it = _channels.find(chanName);
	if (it == _channels.end())
		return true; // channel doesn't exist yet – allowed

	Channel& channel = it->second;

	if (!channel.hasClient(inviterFd))
	{
		sendNumeric(inviterFd, 442, chanName, ":You're not on that channel");
		return false;
	}
	if (channel.isInviteOnly() && !channel.isOperator(inviterFd))
	{
		sendNumeric(inviterFd, 482, chanName, ":You're not channel operator");
		return false;
	}
	if (channel.hasClient(inviteeFd))
	{
		sendNumeric(inviterFd, 443, _clients[inviteeFd].getNickname(), chanName + " :is already on channel");
		return false;
	}
	channel.inviteClient(inviteeFd);
	return true;
}

void Server::processInvite(int inviterFd, int inviteeFd, const std::string& targetNick, const std::string& chanName)
{
	sendNumeric(inviterFd, 341, targetNick, chanName);
	
	Client& inviter = _clients[inviterFd];
    const std::string prefix = ":" + inviter.getNickname() + "!" +
                               inviter.getUsername() + "@" +
                               inviter.getHostname();
    const std::string line = prefix + " INVITE " + targetNick +
                             " :" + chanName + "\r\n";
    sendMsg(inviteeFd, line);
}