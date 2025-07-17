#include "IRC_server.hpp"
#include <algorithm>

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
	else
		sendMessage(fd, "421 " + command + " :Unknown command\r\n");
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

	if (username.empty() || unused1.empty() || unused2.empty() || realname.empty())
	{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters");
		return;
	}

	std::getline(iss, realname); // this includes the colon
	while (!realname.empty() && realname.back() == ' ')
		realname.pop_back();
	if (realname.find(' ') != std::string::npos)
	{	
		if (realname[0] == ':')
		{
			realname.erase(0, 1);
			while (realname[0] == ' ')
				realname.erase(0, 1);
		}
		else 
		{
		sendNumeric(fd, 461, "*", "USER :Not enough parameters (such realname must be prefixed with ':')");
		return;
		}
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
		sendNumeric(client.getFd(), 001, client.getNickname(), "Welcome to the IRC server");
	}
}




void Server::sendMsg(int fd, const std::string& message)
{
	ssize_t sent = send(fd, message.c_str(), message.size(), 0);
	if (sent == -1)
	{
		std::cerr << "Failed to send to fd " << fd << ": " << strerror(errno) << std::endl;//close socket?
	}
}

void Server::sendNumeric(int fd, int code, const std::string& target, const std::string& message)
{
	std::ostringstream oss;
	oss << code << " " << target << " :" << message << "\r\n";
	sendMsg(fd, oss.str());
}

void Server::handlePrivMsg(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handleJoin(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handlePart(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handleKick(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handleQuit(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handleInvite(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handleTopic(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::handleMode(int fd, std::istringstream& iss){(void)fd; std::cout<<iss;}
void Server::sendMessage(int fd, const std::string&iss){(void)fd; std::cout<<iss;}


// void Server::removeClient(int fd)
// {
// 	_clientFds.erase(fd);
// 	close(fd);
// }
