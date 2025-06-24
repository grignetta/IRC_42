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
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>



//#include "OverrExc.hpp"

// int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// struct pollfd {
// 	int   fd;         /* file descriptor */
// 	short events;     /* requested events */
// 	short revents;    /* returned events */
// };


class Server
{
	public:
		Server(int port, const std::string& password);
		~Server();

		void start();

	private:
		int _port;
		std::string _password;
		Socket _serverS;
		std::vector<pollfd> _pollFds;

		void setupSocket();
		//void bindAndListen();
		void acceptNewClient();
};

#endif
