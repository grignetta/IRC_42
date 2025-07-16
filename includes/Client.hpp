#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
	public:
		Client(){}
		Client(int fd);
		int getFd() const;
		
		void setPassApv(bool status);// { _hasGivenPass = status; }
		bool passApv() const;// { return _hasGivenPass; }
		
		const std::string& getNickname() const;
		const std::string& getUsername() const;
		bool isRegistered() const;
		bool isOperator() const;

		void setNickname(const std::string& nick);
		void setUsername(const std::string& user);
		void setRegistered(bool registered);
		void setOperator();
		
		void appendToBuffer(const std::string& data);// { _readBytes += data; }
		std::string& getBuffer();// { return _readBytes; }
		//void sendMessage(const std::string& message);
		
	private:
		int _fd;
		std::string _nickname;
		std::string _username;
		bool _passApv;
		bool _registered;
		bool _operator;
		std::string _readBytes;
};

#endif