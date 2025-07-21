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
	if (!isValidNickname(nick)) {
        throw std::invalid_argument("Invalid IRC nickname: " + nick);
    }
	_nickname = nick;
}

void Client::setUsername(const std::string& user)
{
	if (!isValidUsername(user)) {
        throw std::invalid_argument("Invalid IRC username: " + user);
    }
	_username = user;
}

void Client::setRealname(const std::string& user)
{
	if (user.size() > 50)  //what is max for us?
        throw std::invalid_argument("Realname too long.");
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

namespace {
  const std::string kNickAllowed =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "-[]\\`^{}";

bool isLegalNickFirstChar(char c) {
    return std::isalpha(c) || std::string("[]\\`^{}").find(c) != std::string::npos;
  }
}

bool Client::isValidNickname(const std::string& nick) {
    if (nick.empty() || nick.size() > 9)
        return false;
    if (!isLegalNickFirstChar(nick[0]))
        return false;
    for (std::string::const_iterator it = nick.begin(); it != nick.end(); ++it) {
        if (kNickAllowed.find(*it) == std::string::npos)
            return false;
    }
    return true;
}

bool Client::isValidUsername(const std::string& user) {
    if (user.empty() || user.size() > 10)
        return false;
    for (std::size_t i = 0; i < user.size(); ++i) {
        char c = user[i];
        if (c == ' ' || c == '\r' || c == '\n')
            return false;
    }
    return true;
}