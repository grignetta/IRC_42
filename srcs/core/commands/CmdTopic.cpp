#include "Server.hpp"

void Server::handleTopic(int fd, std::istringstream& iss)
{
	(void)fd; std::cout<<iss;
}
