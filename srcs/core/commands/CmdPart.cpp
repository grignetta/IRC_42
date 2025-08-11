#include "Server.hpp"

// Leave a channel
void Server::handlePart(int fd, std::istringstream& iss) 
{
	// Parse <channel> [:reason…]
	std::vector<std::string> params;
	std::string tok;
	while (iss >> tok) {
		if (tok[0] == ':') {
			std::string rest;
			std::getline(iss, rest);
			tok = tok.substr(1)
				+ (rest.empty() ? "" 
							   : " " + rest.substr((rest[0]==' '||rest[0]==':')?1:0));
			params.push_back(tok);
			break;
		}
		params.push_back(tok);
	}
	if (params.empty()) {
		sendNumeric(fd, 461, "*", "PART :Not enough parameters");
		return;
	}

	std::string chanName = params[0];
	std::string reason   = (params.size()>1 ? params[1] : "");

	// Lookup channel
	std::map<std::string,Channel>::iterator it = _channels.find(chanName);
	if (it == _channels.end())
	{
		sendNumeric(fd, 403, chanName, "No such channel");
		return;
	}
	Channel& ch = it->second;
	if (!ch.hasClient(fd)) {
		sendNumeric(fd, 442, chanName, "You're not on that channel");
		return;
	}

	// Build message prefix
	Client& cl = _clients[fd];
	std::string prefix = ":" + cl.getNickname() + "!"
					   + cl.getUsername() + "@"
					   + cl.getHostname();
	std::string msg = prefix + " PART " + chanName
					+ (reason.empty() ? "" : " :" + reason)
					+ "\r\n";

	// Broadcast to all members
	for (std::map<int,bool>::const_iterator m = ch.getMembers().begin();
		 m != ch.getMembers().end(); ++m)
	{
		sendMsg(m->first, msg);
	}

	// Reassign operator if needed
	if (ch.isOperator(fd)) {
		int opCount = 0;
		for (std::map<int,bool>::const_iterator m = ch.getMembers().begin();
			 m != ch.getMembers().end(); ++m)
		{
			if (ch.isOperator(m->first))
				++opCount;
		}
		if (opCount == 1) {
			for (std::map<int,bool>::const_iterator m = ch.getMembers().begin();
				 m != ch.getMembers().end(); ++m)
			{
				if (m->first != fd) {
					ch.promoteOperator(m->first);
					break;
				}
			}
		}
	}

	ch.removeClient(fd);
	if (ch.getClientCount() == 0)
		_channels.erase(it);
}