#include "Server.hpp"

void Server::sendMsg(int fd, const std::string& message)
{
	std::string msg = message;
	if (msg.size() > 510)
	{
		msg.resize(510);
		msg += "\r\n";
	}
	
	ssize_t sent = send(fd, msg.c_str(), msg.size(), 0);
	if (sent == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			// Client is suspended or output buffer full - drop message
			// This prevents server from hanging and avoids memory leaks
			std::cout << "Dropped message to fd " << fd << " (client suspended/buffer full)" << std::endl;
			return;
		}
		// Other errors (connection lost, etc.)
		std::cerr << "Failed to send to fd " << fd << ": " << strerror(errno) << std::endl;
		// Note: Could call disconnectClient(fd) here for serious errors
	}
	else if (sent < (ssize_t)msg.size())
	{
		// Partial send - for simplicity, we'll drop the remainder
		std::cout << "Partial send to fd " << fd << " (" << sent << "/" << msg.size() << " bytes) - dropping remainder" << std::endl;
	}
}

void Server::sendNumeric(int fd, int code, const std::string& target, const std::string& message)
{
	std::ostringstream oss;
	//oss << code << " " << target << " :" << message << "\r\n";
	oss << ":ircserv " 
		<< std::setw(3) << std::setfill('0') << code  // ensures 001, 002, etc.
		<< " " << target
		<< " " << message 
		<< "\r\n";
	sendMsg(fd, oss.str());
}
