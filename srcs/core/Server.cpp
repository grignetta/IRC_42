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
	else if (command == "PING")//only for weechat
		handlePing(fd, iss);
	else
		sendMsg(fd, ":ircserv 421 " + command + " :Unknown command\r\n");
}

void Server::disconnectClient(int fd)
{
    // Remove client from all channels and collect empty channels
    std::vector<std::string> channelsToRemove;

    for (std::map<std::string, Channel>::iterator chanIt = _channels.begin();
         chanIt != _channels.end(); ++chanIt)
    {
        if (chanIt->second.hasClient(fd)) {
            chanIt->second.removeClient(fd);

            // If channel becomes empty, mark for removal
            if (chanIt->second.getMembers().empty()) {
                channelsToRemove.push_back(chanIt->first);
            }
        }
    }

    // Now safely remove empty channels
    for (std::vector<std::string>::iterator it = channelsToRemove.begin();
         it != channelsToRemove.end(); ++it) {
        _channels.erase(*it);
    }

    // Remove from event loop
    _eventLoop.removeFd(fd);

    // Close the socket
    close(fd);

    // Remove from clients map
    _clients.erase(fd);

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

    // Send quit message to all channels the user is in
    Client& client = _clients[fd];
    if (client.isRegistered()) {
        std::string quitMsg = ":" + client.getNickname() + "!" +
                             client.getUsername() + "@host QUIT :" + reason + "\r\n";

        // Broadcast to all channels user was in
        broadcastQuitToChannels(fd, quitMsg);
    }

    // Close the connection
    disconnectClient(fd);
}

void Server::broadcastQuitToChannels(int fd, const std::string& quitMsg)
{
    // Iterate through all channels to find ones the user was in
    for (std::map<std::string, Channel>::iterator chanIt = _channels.begin();
         chanIt != _channels.end(); ++chanIt)
    {
        Channel& channel = chanIt->second;

        // If the user was in this channel, broadcast quit message to all other members
        if (channel.hasClient(fd)) {
            const std::map<int, bool>& members = channel.getMembers();

            for (std::map<int, bool>::const_iterator memberIt = members.begin();
                 memberIt != members.end(); ++memberIt)
            {
                int memberFd = memberIt->first;

                // Don't send quit message to the user who is quitting
                if (memberFd != fd) {
                    sendMsg(memberFd, quitMsg);
                }
            }
        }
    }
}
