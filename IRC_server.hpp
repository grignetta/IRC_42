#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <cerrno>   // for errno
#include <cstring>  // for strerror

//#include "OverrExc.hpp"

// int poll(struct pollfd *fds, nfds_t nfds, int timeout);

// struct pollfd {
// 	int   fd;         /* file descriptor */
// 	short events;     /* requested events */
// 	short revents;    /* returned events */
// };


#include <string>

class SocketException : public std::exception
{
		std::string _msg;
	public:
		SocketException(const std::string& msg) : _msg(msg) {}
		virtual ~SocketException() throw() {} //why didn't have it in other CPPs
		virtual const char* what() const throw()//why vertual?
		{
			return _msg.c_str();
		}
};

struct Socket
{
	Socket() throw();
	int fd_socket;
	int socket_opt;
	sockaddr_in socket_addr;
};

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
