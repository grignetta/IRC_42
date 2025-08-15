#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <cstdlib>


// MODE #channel
//:ircserv 471 <nick> <channel> 
// 

void Server::handleMode(int fd, std::istringstream& iss)
{
	if (!_clients[fd].isRegistered())
		return sendNumeric(fd, 451, "*", "You have not registered");
	
	std::string chanName;  iss >> chanName;
	if (chanName.empty())
		return sendNumeric(fd, 461, "MODE", ":Not enough parameters");
	
	if (chanName[0] != '#' && chanName[0] != '&')
		return sendNumeric(fd, 403, chanName, ":No such channel");

	Channel* ch = getChannel(chanName);
	if (!ch)
		return sendNumeric(fd, 403, chanName, ":No such channel");
	
	// IF ONLY A TARGE WHAT I MEAN IS it is just MODE #channel then you just tell the modes on
	std::string modeSpec;
	if (!(iss >> modeSpec))
		return sendChannelModeIs(fd, *ch); 
	
	// Only channel operators may change modes
	
	if (!ch->isOperator(fd))
		return sendNumeric(fd, 482, chanName, ":You're not channel operator");
	
	// Collect remaining tokens as parameters
	std::vector<std::string> params;
	for (std::string p; iss >> p; ) params.push_back(p);	

	applyChannelModes(fd, *ch, modeSpec, params); // see next block
}

void Server::sendChannelModeIs(int fd, const Channel& ch)
{
    std::string letters;   // COLLECT i,t,k,l here  LEeading will be added '+')
    std::string params;

    if (ch.isInviteOnly())
        letters += "i";
    if (ch.isTopicRestricted())
        letters += "t";
    if (!ch.getKey().empty()) {
        letters += "k";
        params += " " + ch.getKey();
    }
    if (ch.getUserLimit() > 0) {
        std::ostringstream lim; lim << ch.getUserLimit();
        letters += "l";
        params += " " + lim.str();
    }

    // assemble final payload
    std::string payload;
    if (!letters.empty())
        payload = "+" + letters + params;  // For exp. "+itkl key 20" adding leading +
    else
        payload = "";                      // no modes active

    sendNumeric(fd, 324, ch.getName(), payload);
}


int Server::getClientFdWithNick(const std::string& nick) const
{
    for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getNickname() == nick)
            return it->first; // FD is the map key
    }
    return -1; // not found
}

void Server::applyChannelModes(int fd,
								Channel& ch,
								const std::string& modeSpec,
								const std::vector<std::string>& params)
{

	if (modeSpec[0] != '+' && modeSpec[0] != '-')
	{
		 std::string offending(1, modeSpec[0]);
		sendNumeric(fd, 472, offending, ":is unknown mode char");
		return ;
	}
	if (modeSpec.size() < 2)
		return sendChannelModeIs(fd, ch);
	
	bool plusMode = (modeSpec[0] == '+');
	std::string modesAdded;
	std::size_t paramIndex = 0;

	for (std::size_t i = 0; i < modeSpec.size(); ++i)
	{
		char c = modeSpec[i];
		if (c == '+')
		{
			plusMode = true;
			if (modesAdded.empty() || modesAdded[modesAdded.size() - 1] != '+') modesAdded += '+';
			continue;
		}
		else if (c== '-')
		{
			plusMode = false;
			if (modesAdded.empty() || modesAdded[modesAdded.size() - 1] != '-') modesAdded += '-';
			continue;
		}
		switch (c)
		{
			case 'i':
			{
				
				ch.setInviteOnly(plusMode);
				modesAdded += 'i';
				
			} break;
			case 't':
			{

				ch.setTopicRestricted(plusMode);
				modesAdded += 't';

			} break;
			case 'o':
			{
				if (paramIndex >= params.size())
				{
					sendNumeric(fd, 472, ch.getName(), ":no one in this nickname");
					break;
				}
				int targetFd = getClientFdWithNick(params[paramIndex]);
				if (targetFd == -1) { sendNumeric(fd, 472, ch.getName(), ":no one in this nickname"); paramIndex++; break; } 
				if (!ch.hasClient(targetFd))
    			{
    			    sendNumeric(fd, 472, ch.getName(), ":user not in channel");
    			    paramIndex++;
    			    break;
    			}
				bool changed = false;
    			if (plusMode)
    			{
    			    // avoid duplicate +o
    			    if (!ch.isOperator(targetFd))
    			    {
    			        ch.promoteOperator(targetFd);
    			        changed = true;
    			    }
    			}
    			else
    			{
    			    // avoid duplicate -o
    			    if (ch.isOperator(targetFd))
    			    {
    			        ch.demoteOperator(targetFd);
    			        changed = true;
    			    }
    			}

		    	if (changed)
    			    modesAdded += 'o';
				paramIndex++;
			} break;
			case 'k':
			{
				if (plusMode)
				{
					if (paramIndex < params.size())
					{
						const std::string& key = params[paramIndex];
						ch.setKey(key);
						modesAdded += 'k';
						paramIndex++;
					}else
					{
						sendNumeric(fd, 472, ch.getName(), ":not enough parameter");
						break;
					}

				}else
				{
					ch.setKey("");
					modesAdded += 'k';
				}
			} break;
			case 'l':
			{
				if (plusMode)
				{
					if (paramIndex >= params.size())
                    {
                        sendNumeric(fd, 472, ch.getName(), ":not enough parameter");
                        break;
                    }
					int limit = std::atoi(params[paramIndex].c_str());
					if (limit < 0) limit = 0;
					ch.setUserLimit(limit);
					paramIndex++;
					modesAdded += 'l';
				}
				else
				{
					ch.setUserLimit(0);
					modesAdded += 'l';
				}
			} break;
			default :
			{
				modesAdded += ' ';
				sendNumeric(fd, 472, ch.getName(), ":is unknown mode char");
				break;
			}
		}
	}

	if (!modesAdded.empty())
	{
		Client& client = _clients[fd];
		std::string announce = ":" + client.getNickname() + "!" + client.getUsername() + "@host "
		                      "MODE " + ch.getName() + " " + modesAdded;

		// If you have params that should follow (key, limit, nick, etc.)
		for (size_t i = 0; i < params.size(); ++i)
		    announce += " " + params[i];
		announce += "\n";
		// Broadcast to the whole channel, not just the user
		broadcastToChannel(ch, announce);
	}
}

// static void sendMessage(int fd, std::string line)
// {
//     // Ensure CRLF exactly once
//     if (line.size() < 2 || line.compare(line.size()-2, 2, "\r\n") != 0)
//         line += "\r\n";

//     // IRC hard limit: 512 bytes including CRLF.
//     if (line.size() > 512)
//         line.resize(512);

//     const char* buf = line.c_str();
//     size_t left = line.size();

//     while (left > 0)
//     {
//         ssize_t n = send(fd, buf, left, MSG_NOSIGNAL); // avoid SIGPIPE
//         if (n > 0) { buf += n; left -= static_cast<size_t>(n); continue; }

//         if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
//         {
//             // Non-blocking socket is full — enqueue 'line.substr(line.size()-left)'
//             // to an outgoing buffer and try again later in your event loop.
//             break;
//         }
        // Other errors
//        std::cerr << "sendMsg error to fd " << fd << ": " << strerror(errno);
//        break; // optionally close fd on fatal errors
//    }
	//what
//}

void Server::broadcastToChannel(const Channel& ch, const std::string& message)
{
    for (std::map<int, bool>::const_iterator it = ch.getMembers().begin(); it != ch.getMembers().end(); ++it)
    {
        int memberFd = it->first;
		sendMsg(memberFd, message);
        //sendMessage(memberFd, message); 
		//std::cout << _clients[memberFd].getNickname() << std::endl;
    }
}

// // return true if the change was actually applied; false if rejected (after sending an error)
// bool setInviteOnlyMode(Channel& ch, bool adding);
// bool setTopicRestrictedMode(Channel& ch, bool adding);
// bool setKeyMode(Channel& ch, bool adding, const std::string& key);          // +k needs key, -k ignores it
// bool setOperatorMode(Channel& ch, bool adding, const std::string& nick,     // sets canonical nick to outNick
//                      std::string& outNick);
// bool setLimitMode(Channel& ch, bool adding, const std::string& limitStr);   // +l needs number, -l ignores it