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

// struct sockaddr_in {
//     short            sin_family;   // e.g. AF_INET
//     unsigned short   sin_port;     // e.g. htons(3490)
//     struct in_addr   sin_addr;     // see struct in_addr, below
//     char             sin_zero[8];  // zero this if you want to
// };

// struct in_addr {
//     unsigned long s_addr;  // load with inet_aton()
// };