#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include "Socket.hpp"
#include "Exception.hpp"
#include "Signals.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include <string>
#include <vector>
#include <poll.h>
#include <set>
#include <map>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

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
		//std::set<int> _clientFds;
		std::map<int, Client> _clients;
		std::map<std::string, Channel> _channels;

		void setupSocket();
		void acceptNewClient();
		void handleClientMsg(int fd);
		void parseAndExecCmd(int fd, const std::string& line);
		// void removeClient(int clientFd);
};

#endif
