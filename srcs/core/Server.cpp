#include "Server.hpp"
#include <algorithm>

std::string parseTrailing(std::istream& iss);

Server::Server(int port, const std::string& password) : _port(port), _password(password), _serverS()//why -1?
{
	setupSocket();
	std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{
	if (_serverS.fd_socket != -1)
	{
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			close(it->first); // the FD
		_clients.clear();
		std::cerr << "Server cleaned up.\n";
	}
}

void Server::setupSocket()
{
	_serverS.fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverS.fd_socket < 0)
		throw SocketException(std::string("Failed to create socket: ") + strerror(errno));

	setsockopt(_serverS.fd_socket, SOL_SOCKET, SO_REUSEADDR, &_serverS.socket_opt, sizeof(_serverS.socket_opt));

	std::memset(&_serverS.socket_addr, 0, sizeof(_serverS.socket_addr));
	_serverS.socket_addr.sin_family = AF_INET;
	_serverS.socket_addr.sin_addr.s_addr = INADDR_ANY;
	_serverS.socket_addr.sin_port = htons(_port);

	if (bind(_serverS.fd_socket, (struct sockaddr*)&_serverS.socket_addr, sizeof(_serverS.socket_addr)) < 0)
		throw SocketException(std::string("Failed to bind socket: ") + strerror(errno));

	if (listen(_serverS.fd_socket, 10) < 0)
		throw SocketException(std::string("Failed to listen: ") + strerror(errno));

	fcntl(_serverS.fd_socket, F_SETFL, O_NONBLOCK);// Set the server socket to non-blocking mode// should be changed to write as per subjedt?
	std::cout << "Socket setup complete, listening on port " << _port << std::endl;
}

void Server::start()
{
	_eventLoop.setup(_serverS.fd_socket);

	while (!g_signal)
	{
		std::vector<int> readyFds = _eventLoop.wait();
		if (readyFds.empty())
			continue;
	
		for (std::vector<int>::size_type i = 0; i < readyFds.size(); ++i)
		{
			int fd = readyFds[i];
			if (fd == _serverS.fd_socket)
			{
				acceptNewClient();
			}
			else
			{
				handleClientMsg(fd);
			}
		}
	}
}

void Server::acceptNewClient()
{
	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientSocket = accept(_serverS.fd_socket, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
	
	if (clientSocket < 0)
	{
		std::cerr << "accept() failed: " << strerror(errno) << "\n";
		return;
	}

	// Make non-blocking
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl() failed: " << strerror(errno) << "\n";
		// continue anyway
	}

	// Turn the client IP into a string
	char hostbuf[INET_ADDRSTRLEN];
	if (!inet_ntop(AF_INET, &clientAddr.sin_addr, hostbuf, sizeof(hostbuf)))
	{
		std::cerr << "inet_ntop() failed: " << strerror(errno) << "\n";
		// fallback to empty string
		hostbuf[0] = '\0';
	}

	// Construct Client with fd + hostname
	_clients[clientSocket] = Client(clientSocket, std::string(hostbuf));
	_eventLoop.addFd(clientSocket);

	std::cout << "Accepted new client on fd=" << clientSocket << " from " << hostbuf << "\n";
}


int Server::getPort() const
{
	return _port;
}

void Server::handleClientMsg(int fd)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		//disconnectClient(fd); // EOF or error
		return;
	}
	buffer[bytes] = '\0';
	Client& client = _clients[fd];
	client.appendToBuffer(buffer);

	std::string& buf = client.getBuffer();
	size_t pos;
	//while ((pos = buf.find("\r\n")) != std::string::npos)
	while ((pos = buf.find("\n")) != std::string::npos) //ask why \r\n?
	{
		std::string line = buf.substr(0, pos);
		//buf.erase(0, pos + 2); // remove processed line
		buf.erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')//remove later as we need \r\n
			line.erase(line.size() - 1);//remove later as we need \r\n
		if (line.length() > 510)
		{
			sendNumeric(fd, 421, "*", "<command> :Unknown command");//417 line toot long doesn't exist anymore
			continue; // Skip processing this line
		}	
		parseAndExecCmd(fd, line); // NEXT STEP
	}
}

void Server::parseAndExecCmd(int fd, const std::string& line)
{
	std::istringstream iss(line);
	std::string command;
	iss >> command;
	
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
	if (command == "PASS")
		handlePass(fd, iss);
	else if (command == "NICK")
		handleNick(fd, iss);
	else if (command == "USER")
		handleUser(fd, iss);
	else if (command == "PRIVMSG")
		handlePrivMsg(fd, iss);
	else if (command == "JOIN")
		handleJoin(fd, iss);
	else if (command == "PART")
		handlePart(fd, iss);
	else if (command == "KICK")
		handleKick(fd, iss);
	//else if (command == "QUIT")
		//handleQuit(fd, iss);
	else if (command == "INVITE")
		handleInvite(fd, iss);
	else if (command == "TOPIC")
		handleTopic(fd, iss);
	else if (command == "MODE")
		handleMode(fd, iss);
	else if (command == "PING")//only for weechat
		handlePing(fd, iss);
	else
		sendMsg(fd, "421 " + command + " :Unknown command\r\n");
}

void Server::handlePass(int fd, std::istringstream& iss)
{
	std::string password;
	iss >> password;

	Client& client = _clients[fd];
	if (client.passApv())  // Add a flag in Client class to track this
	{
		sendNumeric(fd, 462, client.getNickname(), "You may not reregister"); // ERR_ALREADYREGISTERED
		return;
	}
	if (password.empty())
	{
		sendNumeric(fd, 461, "*", "PASS :Not enough parameters"); // ERR_NEEDMOREPARAMS "<client> <command> :Not enough parameters" send error differently?
		return;
	}
	if (password != _password)
	{
		sendNumeric(fd, 464, "*", "Password incorrect"); // ERR_PASSWDMISMATCH
		return;
	}
	client.setPassApv(true); // New method in Client
}

void Server::handleNick(int fd, std::istringstream& iss)
{
	std::string nickname;
	iss >> nickname;

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
	Client& client = _clients[fd];
	client.setNickname(nickname);
	checkRegistration(client);
}

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
	// optional: client.setRealname(realname); // if you add realname field
	checkRegistration(client);
}

void Server::checkRegistration(Client& client)
{
	if (client.isRegistered())
		return;

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


void Server::sendMsg(int fd, const std::string& message)
{
	ssize_t sent = send(fd, message.c_str(), message.size(), 0);
	if (sent == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// Output buffer full — either drop or enqueue for retry
			return;
		}
		std::cerr << "Failed to send to fd " << fd << ": " << strerror(errno) << std::endl;//close socket?
	}
}

void Server::sendNumeric(int fd, int code, const std::string& target, const std::string& message)
{
	std::ostringstream oss;
	//oss << code << " " << target << " :" << message << "\r\n";
	oss << ":ircserv " 
		<< std::setw(3) << std::setfill('0') << code  // ensures 001, 002, etc.
		<< " " << target
		<< " " << message 
		<< "\r\n";
	sendMsg(fd, oss.str());
}

void Server::handleJoin(int fd, std::istringstream& iss)
{
	std::string chanName, key;
	iss >> chanName >> key;

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
}

Channel& Server::getOrCreateChannel(const std::string& name, int clientFd)
{
	if (_channels.find(name) == _channels.end())
	{
		_channels[name] = Channel(name);
		_channels[name].addClient(clientFd, true); // first user becomes operator
	}
	return _channels[name];
}

bool Server::tryJoinChannel(int fd, Channel& channel, const std::string& key)//control flags handling
{
	//std::cout << "Client " << fd << " trying to join channel " << channel.getName() << "\n";
	if (channel.isFull())
	{
		sendNumeric(fd, 471, channel.getName(), ":Channel is full");
		return false;
	}
	if (channel.isInviteOnly() && !channel.isInvited(fd))
	{
		sendNumeric(fd, 473, channel.getName(), ":Invite-only channel");
		return false;
	}
	if (channel.hasKey() && channel.getKey() != key)
	{
		sendNumeric(fd, 475, channel.getName(), ":Cannot join channel (+k)");
		return false;
	}
	channel.addClient(fd, false);
	//std::cout << "Client " << fd << " joined channel " << channel.getName() << "\n";
	return true;
}

void Server::announceJoin(Channel& channel, int fd)
{
	Client& client = _clients[fd];
	std::string msg = ":" + client.getNickname() + "!" +
		client.getUsername() + "@localhost JOIN :" + channel.getName() + "\r\n";

	for (std::map<int, bool>::const_iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it)
	{
		sendMsg(it->first, msg);
	}
}

void Server::sendChannelTopic(Channel& channel, int fd)
{
	if (!channel.getTopic().empty())
		sendNumeric(fd, 332, channel.getName(), channel.getTopic());
	else
		sendNumeric(fd, 331, channel.getName(), ":No topic is set");
}

void Server::sendNamesReply(Channel& channel, int fd)
{
	Client& client = _clients[fd];

	// Determine the channel type symbol (we assume public for now)
	std::string visibility = "="; // = public, * private, @ secret

	std::ostringstream oss;
	oss << visibility << " " << channel.getName() << " :";

	for (std::map<int, bool>::const_iterator it = channel.getMembers().begin();
		it != channel.getMembers().end(); ++it)
		{
			Client& member = _clients[it->first];
			if (it->second) // operator flag
				oss << "@" << member.getNickname() << " ";
			else
				oss << member.getNickname() << " ";
		}

	std::string namesLine = oss.str();
	if (!namesLine.empty() && namesLine[namesLine.length() - 1] == ' ')
		namesLine.erase(namesLine.length() - 1); // trim trailing space

	sendNumeric(fd, 353, client.getNickname(), namesLine); // * means the symbol is not important here
}

void Server::handleInvite(int fd, std::istringstream& iss)
{
	std::string targetNick, chanName;
	iss >> targetNick >> chanName;

	if (targetNick.empty() || chanName.empty())// is this check enough?
	{
		sendNumeric(fd, 461, "INVITE", ":Not enough parameters");
		return;
	}

	int inviteeFd = findClient(targetNick);
	if (inviteeFd == -1)
	{
		sendNumeric(fd, 401, targetNick, ":No such nick");
		return;
	}

	if (!invitePerm(fd, inviteeFd, chanName))
		return;

	processInvite(fd, inviteeFd, targetNick, chanName);// sent even if a channel doesn't exist!
}

int Server::findClient(const std::string& nickname) const
{
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.getNickname() == nickname)
			return it->first;
	}
	return -1;
}

bool Server::invitePerm(int inviterFd, int inviteeFd, const std::string& chanName)
{
	std::map<std::string, Channel>::iterator it = _channels.find(chanName);
	if (it == _channels.end())
		return true; // channel doesn't exist yet – allowed

	Channel& channel = it->second;

	if (!channel.hasClient(inviterFd))
	{
		sendNumeric(inviterFd, 442, chanName, ":You're not on that channel");
		return false;
	}
	if (channel.isInviteOnly() && !channel.isOperator(inviterFd))
	{
		sendNumeric(inviterFd, 482, chanName, ":You're not channel operator");
		return false;
	}
	if (channel.hasClient(inviteeFd))
	{
		sendNumeric(inviterFd, 443, _clients[inviteeFd].getNickname(), ":is already on channel");
		return false;
	}
	channel.inviteClient(inviteeFd);
	return true;
}

void Server::processInvite(int inviterFd, int inviteeFd, const std::string& targetNick, const std::string& chanName)
{
	sendNumeric(inviterFd, 341, targetNick, chanName);
	Client& inviter = _clients[inviterFd];
	const std::string& host = inviter.getHostname();
	std::string msg = ":" + inviter.getNickname() + "!" + inviter.getUsername() +
		"@" + host + " INVITE " + targetNick + " " + chanName + "\r\n";
	sendMsg(inviteeFd, msg);
	sendMsg(inviterFd, msg);
}

void Server::handlePrivMsg(int fd, std::istringstream& iss)
{
	(void)fd; std::cout<<iss;
}

void Server::handleKick(int fd, std::istringstream& iss)
{
	std::string chanName, targetNick, comment;
	iss >> chanName >> targetNick;

	if (!verifyKickParams(fd, chanName, targetNick))
		return;
	
	try
	{
		comment = parseTrailing(iss);
	}
	catch (const std::exception& e)
	{
		sendNumeric(fd, 461, "KICK", e.what()); // 461: ERR_NEEDMOREPARAMS
		return;
	}

	Channel* channel = getChannel(chanName);
	if (!channel)
	{
		sendNumeric(fd, 403, chanName, ":No such channel");
		return;
	}
	int targetFd = findClient(targetNick);
	if (!permitKick(fd, *channel, targetNick, targetFd))
		return;
	execKick(*channel, fd, targetNick, comment, targetFd);
}

bool Server::verifyKickParams(int fd, const std::string& chanName, const std::string& targetNick)
{
	if (chanName.empty() || targetNick.empty())
	{
		sendNumeric(fd, 461, "KICK", ":Not enough parameters");
		return false;
	}
	return true;
}

bool Server::permitKick(int fd, Channel& channel, const std::string& targetNick, int targetFd)
{
	if (!channel.hasClient(fd))
	{
		sendNumeric(fd, 442, channel.getName(), ":You're not on that channel");
		return false;
	}
	if (!channel.isOperator(fd))
	{
		sendNumeric(fd, 482, channel.getName(), ":You're not channel operator");
		return false;
	}
	if (targetFd == -1 || !channel.hasClient(targetFd))
	{
		sendNumeric(fd, 441, targetNick, channel.getName() + " :They aren't on that channel");
		return false;
	}
	return true;
}

void Server::execKick(Channel& channel, int kickerFd, const std::string& targetNick, const std::string& comment, int targetFd)//hope 5 are still fine?
{
	Client& kicker = _clients[kickerFd];
	std::string actualComment = comment.empty() ? kicker.getNickname() : comment;
	std::string msg = ":" + kicker.getNickname() + "!" + kicker.getUsername() +
		"@localhost KICK " + channel.getName() + " " + targetNick + " :" + actualComment + "\r\n";// replace localhost with real hostname! Ask Deniz

	for (std::map<int, bool>::const_iterator it = channel.getMembers().begin();
		 it != channel.getMembers().end(); ++it)
	{
		sendMsg(it->first, msg);
	}
	channel.removeClient(targetFd);
}

std::string parseTrailing(std::istream& iss)
{
	std::string trailing;
	std::getline(iss, trailing);

	// Trim leading spaces
	while (!trailing.empty() && trailing[0] == ' ')
		trailing.erase(0, 1);
	if (trailing.empty())
		return "";

	if (trailing.find(' ') != std::string::npos && trailing[0] != ':')
			throw std::runtime_error("Missing ':' before trailing parameter with spaces");//not sure if needed
	// If colon is present, remove it
	if (trailing[0] == ':')
	{
		trailing.erase(0, 1);
		// Remove extra spaces after colon
		while (!trailing.empty() && trailing[0] == ' ')
			trailing.erase(0, 1);
	}
	// Trim trailing spaces
	while (!trailing.empty() && trailing[trailing.size() - 1] == ' ')
		trailing.erase(trailing.size() - 1);
	return trailing;
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return &it->second;
	return NULL;
}

void Server::handleTopic(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;
}

void Server::handleMode(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;
}


// Leave a channel
void Server::handlePart(int fd, std::istringstream& iss) {
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

//void Server::handleQuit(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;
//}

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
}

