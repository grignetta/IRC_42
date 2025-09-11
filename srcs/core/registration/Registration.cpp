#include "Server.hpp"

void Server::checkRegistration(Client& client)
{
	//if (client.isRegistered())
		//return;

	if (client.passApv() &&
		!client.getNickname().empty() &&
		!client.getUsername().empty())
	{
		client.setRegistered(true);
		std::string identity = client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname();
		std::string welcomeMsg = ":Welcome to the Internet Relay Network " + identity;
		sendNumeric(client.getFd(), 001, client.getNickname(), welcomeMsg);
		//sendNumeric(client.getFd(), 001, client.getNickname(), "Welcome to the Internet Relay Network\r\n");//"Welcome to the Internet Relay Network <nick>!<user>@<host>" to update
		//sendNumeric(client.getFd(), 001)
	}
} 
