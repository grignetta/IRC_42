#ifndef SERVER_HPP
#define SERVER_HPP


#ifdef __linux__
	#include "EpollLoop.hpp"
#elif defined(__APPLE__)
	#include "PollLoop.hpp"
#endif


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
        // --- Member Variables ---
        int									_port;
        std::string							_password;
        Socket								_serverS;
        std::map<int, Client>				_clients;
        std::map<std::string, Channel>		_channels;

		#ifdef __linux__
			EpollEventLoop _eventLoop;
		#elif defined(__APPLE__)
			PollEventLoop _eventLoop;
		#endif
        // std::vector<pollfd> _pollFds; // This should be part of your event loop class

        // --- Core Server Logic ---
        void setupSocket();
        void acceptNewClient();
        void handleClientMsg(int fd);
        void parseAndExecCmd(int fd, const std::string& line);
        
        // --- Command Handlers ---
        void handlePass(int fd, std::istringstream& iss);
        void handleNick(int fd, std::istringstream& iss);
        void handleUser(int fd, std::istringstream& iss);
        void handleJoin(int fd, std::istringstream& iss);
        void handlePrivMsg(int fd, std::istringstream& iss);
        void handleKick(int fd, std::istringstream& iss);
        void handleInvite(int fd, std::istringstream& iss);
        void handleTopic(int fd, std::istringstream& iss);
        void handleMode(int fd, std::istringstream& iss);
        void handlePart(int fd, std::istringstream& iss); // Missing
        void handleQuit(int fd, std::istringstream& iss); // Missing

        // --- Helper Functions ---
        void checkRegistration(Client& client);
        Channel& getOrCreateChannel(const std::string& name, int clientFd);
        bool tryJoinChannel(int fd, Channel& channel, const std::string& key);
        void announceJoin(Channel& channel, int fd);
        void sendTopicAndNames(Channel& channel, int fd);

        // --- Messaging ---
        void sendMsg(int fd, const std::string& message);
        void sendNumeric(int fd, int code, const std::string& target, const std::string& message);
        
        // Note: The methods below seem redundant or misplaced
        // bool passApv(); // Client state, should be in Client class
        // void setPassApv(bool apv); // Client state, should be in Client class
        // void sendMessage(int fd, const std::string&iss); // Duplicate of sendMsg?
        // void removeClient(int clientFd); // Should be implemented for disconnects
};

#endif