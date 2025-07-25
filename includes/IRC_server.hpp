#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include "Socket.hpp"
#include "Exception.hpp"
#include "Signals.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include <string>
#include <vector>
#include <poll.h>
#include <set>
#include <map>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define USERLEN 12 //or move to Client.hpp?

class Server
{
	public:
		Server(int port, const std::string& password);
		~Server();

		void start();
		int getPort() const;

	private:
		int _port;
		std::string _password;
		Socket _serverS;
		std::vector<pollfd> _pollFds;
		//std::set<int> _clientFds;
		std::map<int, Client> _clients;
		std::map<std::string, Channel> _channels;

		void setupSocket();
		void acceptNewClient();
		void handleClientMsg(int fd);
		void parseAndExecCmd(int fd, const std::string& line);
		
		void handlePass(int fd, std::istringstream& iss);
		bool passApv();
		void setPassApv(bool apv);
		
		void checkRegistration(Client& client);
		void handleNick(int fd, std::istringstream& iss);
		void handleUser(int fd, std::istringstream& iss);
		
		void sendMsg(int fd, const std::string& message);
		void sendNumeric(int fd, int code, const std::string& target, const std::string& message);
		
		void handleJoin(int fd, std::istringstream& iss);
		Channel& getOrCreateChannel(const std::string& name, int clientFd);
		bool tryJoinChannel(int fd, Channel& channel, const std::string& key);
		void announceJoin(Channel& channel, int fd);
		void sendTopicAndNames(Channel& channel, int fd);
		
		void handlePrivMsg(int fd, std::istringstream& iss);
		void handleKick(int fd, std::istringstream& iss);
		void handleInvite(int fd, std::istringstream& iss);
		void handleTopic(int fd, std::istringstream& iss);
		void handleMode(int fd, std::istringstream& iss);
		void sendMessage(int fd, const std::string&iss);
		// void removeClient(int clientFd);
};

#endif
