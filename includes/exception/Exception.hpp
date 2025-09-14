#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <cerrno> 
#include <cstring>
#include <exception> 
#include <string>

class SocketException : public std::exception
{
	public:
		SocketException(const std::string& msg) : _msg(msg) {}
		virtual ~SocketException() throw() {}
		virtual const char* what() const throw()
		{
			return _msg.c_str();
		}
	private:
		std::string _msg;
};

#endif