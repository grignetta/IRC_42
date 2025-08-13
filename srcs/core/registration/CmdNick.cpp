#include "Server.hpp"

void Server::handleNick(int fd, std::istringstream& iss)
{
	std::string nickname;
	iss >> nickname;
	
	Client& client = _clients[fd];
	if (!client.passApv())
	{
		sendNumeric(fd, 461, "PASS", ":Not enough parameters"); // ERR_NONICKNAMEGIVEN
		return;
	}
	if (nickname.empty())
	{
		sendNumeric(fd, 431, "*", "No nickname given"); // ERR_NONICKNAMEGIVEN
		return;
	}
	// if (nickname.erroneusNick()) - develop this part
	// {
	// 	sendNumeric(fd, 432, nickname, "Erroneous nickname"); // ERR_ERRONEUSNICKNAME
	// 	return;
	// }
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname && it->first != fd)
		{
			sendNumeric(fd, 433, nickname, "Nickname is already in use"); // ERR_NICKNAMEINUSE
			return;
		}
	}
	client.setNickname(nickname);
	//I did this Change here and in USER because my registeration was always giving an error
	// client.incrementRegisterNickUserNames(1);
	// if (client.getRegisterNickUserNames() == 2)
	// 	client.setRegistered(true);
	
	checkRegistration(client);
}