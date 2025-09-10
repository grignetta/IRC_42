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
		
		// when this is 2 set registed to true // just an idea open to suggestions
		int registerNickUserNames;

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
		const int& getRegisterNickUserNames() const;

		//Bools
		bool isRegistered() const;
		bool isOperator() const;


		//Setters
		bool setNickname(const std::string& nick);
		bool setUsername(const std::string& user);
		bool setRealname(const std::string& user);
		void setRegistered(bool registered);
		void setOperator();
		void incrementRegisterNickUserNames(int increment);
		
		void appendToBuffer(const std::string& data);// { _readBytes += data; }
		std::string& getBuffer();// { return _readBytes; }
		void sendMessage(const std::string& message);
};

#endif