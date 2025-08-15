#include "Server.hpp"

void Server::handlePrivMsg(int fd, std::istringstream& iss)
{
	// Must be registered first
	// CHECKING BASIC IF IT IS REGISTERD AND IF TARGET IS EMPTY OR NOT
	Client& sender = _clients[fd];
	if (!sender.isRegistered())
	{
		sendNumeric(fd, 451, "*", ":You have not registered");
		return;
	}

	// Parse targets
	std::string targets;
	iss >> targets;
	if (targets.empty())
	{
		sendNumeric(fd, 411, "*", ":No recipient given (PRIVMSG)");
		return;
	}

	// Parse the trailing text (allows spaces), using our helper USING THE
	// PARSE TRAILING WHICH DARIA CREATED
	std::string text;
	try
	{
		text = parseTrailing(iss);
	} catch (...)
	{
		// Missing ':' before spaced text, etc.
		sendNumeric(fd, 412, "*", ":No text to send");
		return;
	}
	if (text.empty())
	{
		sendNumeric(fd, 412, "*", ":No text to send");
		return;
	}

	// Building sender prefix once to identify ON THE RECIEVER end
	// :newUser!newUser@127.0.0.1 PRIVMSG ncUser :Message
	const std::string prefix = ":" + sender.getNickname() + "!" + sender.getUsername() + "@" + sender.getHostname();

	// Support comma-separated targets THIS IS IMPORTTANT IF WE SEND THE MESSAGE TO MORE THAN ONE RECIEPENTS
	size_t start = 0;
	while (start < targets.size())
	{
		size_t comma = targets.find(',', start);
		std::string t = targets.substr(start, (comma == std::string::npos ? targets.size() : comma) - start);
		start = (comma == std::string::npos ? targets.size() : comma + 1);
		if (t.empty())
			continue;	
		if (t[0] == '#' || t[0] == '&')
		{
			// Channel target
			Channel* ch = getChannel(t);
			if (!ch)
			{
				sendNumeric(fd, 403, t, ":No such channel");
				continue;
			}
			// IT WAS NOT EXPLICITLY SAID ABOUT +N IN THE MODES SO I AM ACTING AS +N IS ALWAYS ON SO NON CHANNEL MEMBERS CAN NOT SEND
			// MESSEAGE TO THE CHANNEL
			if (!ch->hasClient(fd))
			{
				sendNumeric(fd, 404, t, ":Cannot send to channel");
				continue;
			}
			const std::string out = prefix + " PRIVMSG " + t + " :" + text + "\r\n";
			for (std::map<int,bool>::const_iterator it = ch->getMembers().begin(); it != ch->getMembers().end(); ++it)
			{
				if (it->first == fd) continue;   // keps the message away from sender// not send the message back to sender
				sendMsg(it->first, out);
			}
		}
		else
		{
			// Nickname target
			int rcptFd = findClient(t);
			if (rcptFd == -1)
			{
				sendNumeric(fd, 401, t, ":No such nick/channel");
				continue;
			}
			const std::string out = prefix + " PRIVMSG " + t + " :" + text + "\r\n";
			sendMsg(rcptFd, out);
		}
	}
}
