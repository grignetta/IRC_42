#include "Server.hpp"

bool controlPort(char* argv)
{   
	std::string portStr = argv;
	if (portStr.length() > 5)
	{
		std::cerr << "Invalid port: must be 1–65535\n";
		return false;
	}
	int port = std::atoi(portStr.c_str());
	if (port < 1 || port > 65535)
	{
		std::cerr << "Invalid port: must be 1–65535\n";
		return false;
	}
	return true;
}

bool controlPassword(std::string& password)
{
	if (password.empty() || password.length() > 64)
		return false;

	for (size_t i = 0; i < password.size(); ++i)
	{
		if (std::isspace(static_cast<unsigned char>(password[i])))
		{
			return false;
		}
	}
	return true;
}