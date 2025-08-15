#include "Server.hpp"

static bool isJoinZeroToken(std::string s) {
    while (!s.empty() && (s[0] == ' ' || s[0] == '\t')) s.erase(s.begin());
    while (!s.empty() && (s[s.size()-1] == ' ' || s[s.size()-1] == '\t')) s.erase(s.end()-1);
    return s == "0";
}

void Server::handleJoin(int fd, std::istringstream& iss)
{
	std::string chanName, key;
	iss >> chanName >> key;

	//  JOIN 0: I LEAVE all channels and return -----
    if (isJoinZeroToken(chanName)) {
        leaveAllChannels(fd, "Leaving all channels");
        return;
    }

	if (!Channel::isValidName(chanName))
	{
		sendNumeric(fd, 476, "*", chanName + " :Invalid channel name");
		return;
	}

	Channel& channel = getOrCreateChannel(chanName, fd);
	
	if (!channel.hasClient(fd) && !tryJoinChannel(fd, channel, key))
		return;
	//if (channel.getClientCount() >= 1 && !tryJoinChannel(fd, channel, key))
		//return;
	
	announceJoin(channel, fd);
	sendChannelTopic(channel, fd);
	sendNamesReply(channel, fd);
	std::string msg = ":ircserv 366 " + _clients[fd].getNickname() + " "+ chanName + ":End of NAMES list";
	broadcastToChannel(channel, msg);
}

void Server::leaveAllChannels(int fd, const std::string& reason)
{
    // Snapshot names first to avoid iterator invalidation while PARTing.
    std::vector<std::string> toLeave;
    for (std::map<std::string, Channel>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
    {
        if (it->second.hasClient(fd))
            toLeave.push_back(it->first);
    }

    // REUSE OF PART handler for each channel: "PART #chan :reason"
    for (size_t i = 0; i < toLeave.size(); ++i)
    {
        const std::string &name = toLeave[i];
        std::ostringstream oss;
        oss << name;
        if (!reason.empty())
            oss << " :" << reason;

        std::istringstream iss(oss.str());
        handlePart(fd, iss);
    }
}
//Note that this message accepts a special argument ("0"), which is
//    a special request to leave all channels the user is currently a member
//    of.  The server will process this message as if the user had sent
//    a PART command (See Section 3.2.2) for each channel he is a member
//    of.  ??????????