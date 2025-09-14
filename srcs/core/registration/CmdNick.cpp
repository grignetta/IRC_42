#include "Server.hpp"

void Server::handleNick(int fd, std::istringstream& iss)
{
	std::string nickname;
	iss >> nickname;
	
	Client& client = _clients[fd];
	if (client.isRegistered())
	{
		sendNumeric(fd, 462, client.getNickname(), ":You may not reregister"); 
		return;
	}
	if (!client.passApv())
	{
		sendNumeric(fd, 451, "*", ":You have not registered"); // ERR_NONICKNAMEGIVEN
		return;
	}
	if (nickname.empty())
	{
		sendNumeric(fd, 431, "*", ":No nickname given"); // ERR_NONICKNAMEGIVEN
		return;
	}
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname && it->first != fd)
		{
			sendNumeric(fd, 433, nickname, "Nickname is already in use"); // ERR_NICKNAMEINUSE
			return;
		}
	}
	
	if (!client.setNickname(nickname)) {
		sendNumeric(fd, 432, nickname, ":Erroneous nickname");
		return;
	}
	
	//checkRegistration(client);
}