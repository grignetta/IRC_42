#include "Server.hpp"
/*
void Server::handlePing(int fd, std::istringstream& iss)
{
	std::string target;
	iss >> target;

	if (target.empty())
	{
		sendNumeric(fd, 409, "*", "No target specified for PING");
		return;
	}

	std::string response = ":ircserv PONG " + target + "\r\n";
	sendMsg(fd, response);
}*/
