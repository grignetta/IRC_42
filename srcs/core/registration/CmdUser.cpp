#include "Server.hpp"

void Server::handleUser(int fd, std::istringstream& iss)
{
	std::string username, unused1, unused2, realname;
	iss >> username >> unused1 >> unused2;

	if (username.empty() || unused1.empty() || unused2.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters");
		return;
	}

	std::getline(iss, realname); // this includes the colon
	
	// Trim leading and trailing spaces
	while (!realname.empty() && realname[0] == ' ')
		realname.erase(0, 1);
	while (!realname.empty() && realname[realname.size() - 1] == ' ')
		realname.erase(realname.size() - 1);
		
	if (!realname.empty() && realname[0] == ':')
	{
		realname.erase(0, 1);
		while (!realname.empty() && realname[0] == ' ') //can this happen?
			realname.erase(0, 1);
	}
	else if (!realname.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters (realname must be prefixed with ':')");
		return;
	}
	
	Client& client = _clients[fd];
	if (!client.getUsername().empty())
	{
		sendNumeric(fd, 462, client.getNickname(), "You may not reregister"); // ERR_ALREADYREGISTERED
		return;
	}
	// ADD username control USERLEN
	client.setUsername(username);
	//I did this Change here and in NICK because my registeration was always giving an error
	client.incrementRegisterNickUserNames(1);
	if (client.getRegisterNickUserNames() == 2)
		client.setRegistered(true);
	// optional: client.setRealname(realname); // if you add realname field
	checkRegistration(client);
}