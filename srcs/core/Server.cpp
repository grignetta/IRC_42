#include "Server.hpp"
#include <algorithm>

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

	if (listen(_serverS.fd_socket, SOMAXCONN) < 0)
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

	int clientSocket = accept(_serverS.fd_socket, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);//always an fd

	if (clientSocket < 0)
	{
		std::cerr << "accept() failed: " << strerror(errno) << "\n";
		return;
	}

	// Make non-blocking
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "fcntl() failed: " << strerror(errno) << "\n";
	}

	char hostbuf[INET_ADDRSTRLEN];// client IP -> a string
	if (!inet_ntop(AF_INET, &clientAddr.sin_addr, hostbuf, sizeof(hostbuf)))
	{
		std::cerr << "inet_ntop() failed: " << strerror(errno) << "\n";
		hostbuf[0] = '\0';
	}

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
		disconnectClient(fd); // EOF or error
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
			continue;
		}
		parseAndExecCmd(fd, line);
		if (_clients.find(fd) == _clients.end())
			return; // Client might have been disconnected during command processing
	}
}

void Server::parseAndExecCmd(int fd, const std::string& line)
{
	std::istringstream iss(line);
	std::string command;
	iss >> command;

	std::transform(command.begin(), command.end(), command.begin(), ::toupper);
	if (command.empty())
		return;
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
	else if (command == "QUIT")
		handleQuit(fd, iss);
	else if (command == "INVITE")
		handleInvite(fd, iss);
	else if (command == "TOPIC")
		handleTopic(fd, iss);
	else if (command == "MODE")
		handleMode(fd, iss);
	//else if (command == "PING")//only for weechat
		//handlePing(fd, iss);
	else
		sendMsg(fd, ":ircserv 421 " + command + " :Unknown command\r\n");
}

void Server::disconnectClient(int fd)
{
	// Remove client from all channels first
	for (std::map<std::string, Channel>::iterator chanIt = _channels.begin();
		 chanIt != _channels.end(); ++chanIt)
	{
		if (chanIt->second.hasClient(fd))
		{
			chanIt->second.removeClient(fd);

			// If channel becomes empty, remove
			if (chanIt->second.getMembers().empty())
			{
				_channels.erase(chanIt);
				break;
			}
		}
	}
	_eventLoop.removeFd(fd); // Remove from event loop
	close(fd); // Close the socket
	_clients.erase(fd); // Remove from clients map
	std::cout << "Disconnected client fd=" << fd << std::endl;
}

void Server::handleQuit(int fd, std::istringstream& iss)
{
	std::string reason;
	std::getline(iss, reason);

	if (!_clients[fd].isRegistered()) {
		sendNumeric(fd, 451, "*", ":You have not registered");
		return;
	}
	
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		Channel& channel = it->second;
		if (channel.hasClient(fd) && channel.isOperator(fd) && channel.countOperators() == 1)
		{
			std::vector<int> toKick;
			const std::map<int, bool>& members = channel.getMembers();
			for (std::map<int, bool>::const_iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
				if (memberIt->first != fd)
					toKick.push_back(memberIt->first);
			}
			for (size_t i = 0; i < toKick.size(); ++i) {
				int memberFd = toKick[i];
				std::string targetNick = _clients[memberFd].getNickname();
				execKick(channel, fd, targetNick, "No operators left in channel", memberFd);
			}
		}
	}

	Client& client = _clients[fd];
	if (client.isRegistered())
	{
		std::string quitMsg = ":" + client.getNickname() + "!" +
							 client.getUsername() + "@" + client.getHostname() + " QUIT :" + reason + "\r\n";
		broadcastQuitToChannels(fd, quitMsg);//to all channels user was in
	}
	disconnectClient(fd);
}

void Server::broadcastQuitToChannels(int fd, const std::string& quitMsg)
{
	for (std::map<std::string, Channel>::iterator chanIt = _channels.begin();//find channels user was in
		 chanIt != _channels.end(); ++chanIt)
	{
		Channel& channel = chanIt->second;
		if (channel.hasClient(fd))
		{
			const std::map<int, bool>& members = channel.getMembers();

			for (std::map<int, bool>::const_iterator memberIt = members.begin();
				 memberIt != members.end(); ++memberIt)
			{
				int memberFd = memberIt->first;
				if (memberFd != fd)// not to the user who quit
				{
					sendMsg(memberFd, quitMsg);
				}
			}
		}
	}
}
