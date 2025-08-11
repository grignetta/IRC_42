#include "Server.hpp"

int Server::findClient(const std::string& nickname) const
{
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
			return it->first;
	}
	return -1;
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return &it->second;
	return NULL;
}
