#include "Server.hpp"

void Server::handleMode(int fd, std::istringstream& iss)
{
	(void)fd; std::cout<<iss;
}