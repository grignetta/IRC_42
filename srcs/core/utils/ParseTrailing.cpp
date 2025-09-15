#include "Server.hpp"

std::string  Server::parseTrailing(std::istream& iss)
{
	std::string trailing;
	std::getline(iss, trailing);

	// Trim leading spaces
	while (!trailing.empty() && trailing[0] == ' ')
		trailing.erase(0, 1);
	if (trailing.empty())
		return "";

	if (trailing.find(' ') != std::string::npos && trailing[0] != ':')
			throw std::runtime_error("Missing ':' before trailing parameter with spaces");
	// If colon is present, remove it
	if (trailing[0] == ':')
	{
		trailing.erase(0, 1);
		// Remove extra spaces after colon
		while (!trailing.empty() && trailing[0] == ' ')
			trailing.erase(0, 1);
	}
	// Trim trailing spaces
	while (!trailing.empty() && trailing[trailing.size() - 1] == ' ')
		trailing.erase(trailing.size() - 1);
	return trailing;
}
