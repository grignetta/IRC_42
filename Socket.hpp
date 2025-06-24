#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>

struct Socket
{
	Socket() throw();
	int fd_socket;
	int socket_opt;
	sockaddr_in socket_addr;
};

#endif