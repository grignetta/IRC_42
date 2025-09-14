#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <stdexcept>

class Client
{
	private:
		int _fd;
		std::string _nickname;
		std::string _username;
		std::string _realname;
		std::string _hostname;    
		
		bool _passApv;
		bool _registered;
		bool _operator;

		std::string _readBytes;

		static bool isValidNickname(const std::string& nick);
  		static bool isValidUsername(const std::string& user);
	
	public:
		Client(){}
		Client(int fd, const std::string& host);
		int getFd() const;
		
		void setPassApv(bool status);// { _hasGivenPass = status; }
		bool passApv() const;// { return _hasGivenPass; }
		
		
		//Getters	
		const std::string& getNickname() const;
		const std::string& getUsername() const;
		const std::string& getRealname() const;
		const std::string& getHostname() const;

		//Bools
		bool isRegistered() const;
		bool isOperator() const;


		//Setters
		bool setNickname(const std::string& nick);
		bool setUsername(const std::string& user);
		bool setRealname(const std::string& user);
		void setRegistered(bool registered);
		
		void appendToBuffer(const std::string& data);// { _readBytes += data; }
		std::string& getBuffer();// { return _readBytes; }
};

#endif