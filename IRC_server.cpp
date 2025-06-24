#include "IRC_server.hpp"

Socket::Socket() throw() : fd_socket(-1), socket_opt(1), socket_addr() {}

Server::Server(int port, const std::string& password) : _port(port), _password(password), _serverS()//why -1?
{
	setupSocket();
	std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{
	if (_serverS.fd_socket != -1)
		close(_serverS.fd_socket);
}

void Server::setupSocket()
{
	_serverS.fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverS.fd_socket < 0)
		throw SocketException(std::string("Failed to create socket: ") + strerror(errno));
		//throw std::runtime_error("Failed to create socket");

	setsockopt(_serverS.fd_socket, SOL_SOCKET, SO_REUSEADDR, &_serverS.socket_opt, sizeof(_serverS.socket_opt));

	std::memset(&_serverS.socket_addr, 0, sizeof(_serverS.socket_addr));
	_serverS.socket_addr.sin_family = AF_INET;
	_serverS.socket_addr.sin_addr.s_addr = INADDR_ANY;
	_serverS.socket_addr.sin_port = htons(_port);

	if (bind(_serverS.fd_socket, (struct sockaddr*)&_serverS.socket_addr, sizeof(_serverS.socket_addr)) < 0)
		throw SocketException(std::string("Failed to bind socket: ") + strerror(errno));
		//throw std::runtime_error("Failed to bind socket");

	if (listen(_serverS.fd_socket, 10) < 0)
		throw SocketException(std::string("Failed to listen: ") + strerror(errno));
		//throw std::runtime_error("Failed to listen");

	fcntl(_serverS.fd_socket, F_SETFL, O_NONBLOCK);

	pollfd pfd;
	pfd.fd = _serverS.fd_socket;
	pfd.events = POLLIN;
	_pollFds.push_back(pfd);

	std::cout << "Server listening on port " << _port << std::endl;
}

void Server::start()
{
	while (true)
	{
		int ret = poll(&_pollFds[0], _pollFds.size(), -1);
		if (ret < 0)
			throw std::runtime_error("poll() failed");

		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (_pollFds[i].fd == _serverS.fd_socket && (_pollFds[i].revents & POLLIN))
			{
				acceptNewClient();
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

	fcntl(clientSocket, F_SETFL, O_NONBLOCK);

	pollfd newPfd;
	newPfd.fd = clientSocket;
	newPfd.events = POLLIN;
	_pollFds.push_back(newPfd);

	std::cout << "New client connected: " << clientSocket << std::endl;
}
