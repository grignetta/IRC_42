#include "IRC_server.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int port = std::atoi(argv[1]);//is there cpp version?
	std::string password = argv[2];

	try
	{
		Server server(port, password);
		server.start();
		std::cout << "Server listening on port " << server.getPort() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
