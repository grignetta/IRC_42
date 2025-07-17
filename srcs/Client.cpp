#include "Client.hpp"

Client::Client(int fd)
  : _fd(fd)
  , _passApv(false)     // matches first in class
  , _registered(false)  // second
  , _operator(false)    // third
{}

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

const std::string& Client::getRealname() const
{
	return _realname;
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

void Client::setRealname(const std::string& user)
{
	_realname = user;
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

void Client::setPassApv(bool status)
{
	_passApv = status;
}

bool Client::passApv() const
{
	return _passApv;
}

//void setOperator()

// void Client::sendMessage(const std::string& message)
// {
// 		::send(fd, message.c_str(), message.length(), 0);
// }