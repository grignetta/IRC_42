#include "Server.hpp"

void Server::handleUser(int fd, std::istringstream& iss)
{
	std::string username, unused1, unused2, realname;
	iss >> username >> unused1 >> unused2;

	Client& client = _clients[fd];
	if (!client.passApv())
	{
		sendNumeric(fd, 451, "*", ":You have not registered"); // ERR_NONICKNAMEGIVEN
		return;
	}
	if (client.getNickname().empty())
	{
		sendNumeric(fd, 431, "*", ":No nickname given"); 
		return;
	}
	if (username.empty() || unused1.empty() || unused2.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters");
		return;
	}

	std::getline(iss, realname); // this includes the colon
	if (realname.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters");
		return;
	}
	
	// Trim leading and trailing spaces
	while (!realname.empty() && realname[0] == ' ')
		realname.erase(0, 1);
	while (!realname.empty() && realname[realname.size() - 1] == ' ')
		realname.erase(realname.size() - 1);
		
	if (!realname.empty() && realname[0] == ':')
	{
		realname.erase(0, 1);
		while (!realname.empty() && realname[0] == ' ')
			realname.erase(0, 1);
	}
	else if (!realname.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters (realname must be prefixed with ':')");
		return;
	}
	if (realname.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters");
		return;
	}
	
	if (!client.getUsername().empty())
	{
		sendNumeric(fd, 462, client.getNickname(), ":You may not reregister"); // ERR_ALREADYREGISTERED
		return;
	}
	if (!client.setUsername(username)) {
		sendNumeric(fd, 432, username, ":Invalid username");
		return;
	}
	if (!client.setRealname(realname)) {
		sendNumeric(fd, 432, realname, ":Realname too long");
		return;
	}
	checkRegistration(client);
}