#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _registered(false), _operator(false) {}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

bool Client::isRegistered() const
{
	return _registered;
}

void Client::setNickname(const std::string& nick)
{
	_nickname = nick;
}

void Client::setUsername(const std::string& user)
{
	_username = user;
}

void Client::setRegistered(bool reg)
{
	_registered = reg;
}

void Client::appendToBuffer(const std::string& bytes)
{
	_readBytes += bytes;
}
std::string& Client::getBuffer()
{
	return _readBytes;
}

//void setOperator()

// void Client::sendMessage(const std::string& message)
// {
// 		::send(fd, message.c_str(), message.length(), 0);
// }