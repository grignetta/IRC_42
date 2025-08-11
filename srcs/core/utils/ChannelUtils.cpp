#include "Server.hpp"

Channel& Server::getOrCreateChannel(const std::string& name, int clientFd)
{
	if (_channels.find(name) == _channels.end())
	{
		_channels[name] = Channel(name);
		_channels[name].addClient(clientFd, true); // first user becomes operator
	}
	return _channels[name];
}

bool Server::tryJoinChannel(int fd, Channel& channel, const std::string& key)//control flags handling
{
	//std::cout << "Client " << fd << " trying to join channel " << channel.getName() << "\n";
	if (channel.isFull())
	{
		sendNumeric(fd, 471, channel.getName(), ":Channel is full");
		return false;
	}
	if (channel.isInviteOnly() && !channel.isInvited(fd))
	{
		sendNumeric(fd, 473, channel.getName(), ":Invite-only channel");
		return false;
	}
	if (channel.hasKey() && channel.getKey() != key)
	{
		sendNumeric(fd, 475, channel.getName(), ":Cannot join channel (+k)");
		return false;
	}
	channel.addClient(fd, false);
	//std::cout << "Client " << fd << " joined channel " << channel.getName() << "\n";
	return true;
}

void Server::announceJoin(Channel& channel, int fd)
{
	Client& client = _clients[fd];
	std::string msg = ":" + client.getNickname() + "!" +
		client.getUsername() + "@localhost JOIN :" + channel.getName() + "\r\n";

	for (std::map<int, bool>::const_iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it)
	{
		sendMsg(it->first, msg);
	}
}

void Server::sendChannelTopic(Channel& channel, int fd)
{
	if (!channel.getTopic().empty())
		sendNumeric(fd, 332, channel.getName(), channel.getTopic());
	else
		sendNumeric(fd, 331, channel.getName(), ":No topic is set");
}

void Server::sendNamesReply(Channel& channel, int fd)
{
	Client& client = _clients[fd];

	// Determine the channel type symbol (we assume public for now)
	std::string visibility = "="; // = public, * private, @ secret

	std::ostringstream oss;
	oss << visibility << " " << channel.getName() << " :";

	for (std::map<int, bool>::const_iterator it = channel.getMembers().begin();
		it != channel.getMembers().end(); ++it)
		{
			Client& member = _clients[it->first];
			if (it->second) // operator flag
				oss << "@" << member.getNickname() << " ";
			else
				oss << member.getNickname() << " ";
		}

	std::string namesLine = oss.str();
	if (!namesLine.empty() && namesLine[namesLine.length() - 1] == ' ')
		namesLine.erase(namesLine.length() - 1); // trim trailing space

	sendNumeric(fd, 353, client.getNickname(), namesLine); // * means the symbol is not important here
}