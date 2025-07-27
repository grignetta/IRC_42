#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <cerrno>   // for errno
#include <cstring>  // for strerror
#include <exception> // for std::exception
#include <string>   // for std::string

class SocketException : public std::exception
{
	public:
		SocketException(const std::string& msg) : _msg(msg) {}
		virtual ~SocketException() throw() {} //why didn't have it in other CPPs
		virtual const char* what() const throw()//why vertual?
		{
			return _msg.c_str();
		}
	private:
		std::string _msg;
};

#endif