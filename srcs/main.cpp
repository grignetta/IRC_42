#include "Server.hpp"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	
	setupSignalHandlers();

	if (!controlPort(argv[1]))
		return 1;
	int port = std::atoi(argv[1]);
	std::string password = argv[2];
	if (!controlPassword(password))
	{
		std::cerr << "Invalid password: cannot be empty, contain spaces, or exceed 64 characters\n";
		return 1;
	}

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
