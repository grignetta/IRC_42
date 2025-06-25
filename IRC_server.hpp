#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include "Socket.hpp"
#include "Exception.hpp"

#include <string>
#include <vector>
#include <poll.h>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

class Server
{
	public:
		Server(int port, const std::string& password);
		~Server();

		void start();
		int getPort() const;

	private:
		int _port;
		std::string _password;
		Socket _serverS;
		std::vector<pollfd> _pollFds;

		void setupSocket();
		void acceptNewClient();
};

#endif
