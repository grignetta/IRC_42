#include "Server.hpp"

void Server::handlePass(int fd, std::istringstream& iss)
{
	std::string password;
	iss >> password;

	Client& client = _clients[fd];
	if (client.passApv())
	{
		sendNumeric(fd, 462, client.getNickname(), ":You may not reregister"); // ERR_ALREADYREGISTERED
		return;
	}
	if (password.empty())
	{
		sendNumeric(fd, 461, "*", "PASS :Not enough parameters"); // ERR_NEEDMOREPARAMS
		return;
	}
	if (password != _password)
	{
		sendNumeric(fd, 464, "*", "Password incorrect"); // ERR_PASSWDMISMATCH
		return;
	}
	client.setPassApv(true);
}
