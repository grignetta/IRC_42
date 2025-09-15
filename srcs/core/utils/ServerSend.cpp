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
			// Output buffer full — either drop or enqueue for retry
			return;
		}
		std::cerr << "Failed to send to fd " << fd << ": " << strerror(errno) << std::endl;
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
