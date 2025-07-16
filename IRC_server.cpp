#include "IRC_server.hpp"

#ifdef __linux__
#include "EpollLoop.hpp"
#elif defined(__APPLE__)
#include "PollLoop.hpp"
#endif

Server::Server(int port, const std::string& password) : _port(port), _password(password), _serverS()//why -1?
{
	setupSocket();
	std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{
	if (_serverS.fd_socket != -1)
	{
		//for (std::set<int>::iterator it = _clientFds.begin(); it != _clientFds.end(); ++it)
		//close(*it);
		//close(_serverS.fd_socket);
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
	#ifdef __linux__
		EpollEventLoop eventLoop;
	#elif defined(__APPLE__)
		PollEventLoop eventLoop;
	#endif

	eventLoop.setup(_serverS.fd_socket);

	while (!g_signal)
	{
		int ready = eventLoop.wait();//epoll_wait() fills up to ready entries in your events[] array
		if (ready < 0)
			return ;
		for (int i = 0; i < ready; ++i)
		{
			int fd = eventLoop.getReadyFd(i);//_pollFds[index].fd or _events[index].data.fd;
			if (fd == _serverS.fd_socket)// = Is this event from the main server socket (meaning a new client)
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
	int clientSocket = accept(_serverS.fd_socket, (struct sockaddr*)&clientAddr, &clientLen);
	if (clientSocket < 0) {
		std::cerr << "Failed to accept new connection" << std::endl;
		return;
	}
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);// Set the client socket to non-blocking mode// should be changed to write as per subjedt?
	std::cout << "Accepted new client connection" << std::endl;
	_clients[clientSocket] = Client(clientSocket);
	//_clientFds.insert(clientSocket);//delete when client disconnects
}

int Server::getPort() const
{
	return _port;
}

void Server::handleClientMsg(int fd)
{
	char buffer[BUFFER_SIZE]; // probably maloc it and make stretchable?
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
	while ((pos = buf.find("\r\n")) != std::string::npos)
	{
		std::string line = buf.substr(0, pos);
		buf.erase(0, pos + 2); // remove processed line
		parseAndExecCmd(fd, line); // NEXT STEP
	}
}

void Server::parseAndExecCmd(int fd, const std::string& line)
{
	std::istringstream iss(line);
	std::string command;
	iss >> command;
	
	std::transform(command.begin(), command.end(), command.begin(), ::toupper);

	if (command == "NICK")
		handleNick(fd, iss);
	else if (command == "USER")
		handleUser(fd, iss);
	//else if (command == "PASS") - not sure it's neeeded
	//handlePass(fd, iss);
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
	else
		sendMessage(fd, "421 " + command + " :Unknown command\r\n");
}

// void Server::removeClient(int fd)
// {
// 	_clientFds.erase(fd);
// 	close(fd);
// }
