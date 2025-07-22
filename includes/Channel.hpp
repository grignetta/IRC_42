#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>

class Channel
{
	std::string _name;
	std::string _topic;
	std::set<int> _clients;       // FDs of users
	std::set<int> _operators;     // FDs of ops
	bool _inviteOnly __attribute__((unused));
	bool _topicRestricted __attribute__((unused));
	std::string _key;
	int _userLimit __attribute__((unused));
	std::set<int> _invited;       // For +i mode

	// Methods: addClient(), removeClient(), isOperator(), etc.
};

#endif