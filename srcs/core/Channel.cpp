#include "Channel.hpp"

Channel::Channel() : _name(""), _inviteOnly(false), _topicRestricted(false), _userLimit(0) {} //not sure why without this one there's a compiler error


Channel::Channel(const std::string& name)
	: _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(0) {}

Channel::~Channel() {}// Add some cleanup if needed

void Channel::addClient(int fd, bool isOperator = false)
{
	_members[fd] = isOperator;
}

void Channel::removeClient(int fd)
{
	_members.erase(fd);
	//_invited.erase(fd); 
}

bool Channel::hasClient(int fd) const
{
	return _members.find(fd) != _members.end();
}

int Channel::getClientCount() const
{
	return _members.size();
}

bool Channel::isOperator(int fd) const
{
	std::map<int, bool>::const_iterator it = _members.find(fd);
	return it != _members.end() && it->second;
}

void Channel::promoteOperator(int fd)
{
	if (hasClient(fd))
		_members[fd] = true;
}

void Channel::demoteOperator(int fd)
{
	if (hasClient(fd))
		_members[fd] = false;
}

const std::map<int, bool>& Channel::getMembers() const
{
	return _members;
}

void Channel::setTopic(const std::string& topic) {
	_topic = topic;
}

const std::string& Channel::getTopic() const {
	return _topic;
}

void Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}

bool Channel::isTopicRestricted() const
{
	return _topicRestricted;
}

void Channel::setInviteOnly(bool value) {
	_inviteOnly = value;
}

bool Channel::isInviteOnly() const {
	return _inviteOnly;
}

void Channel::setKey(const std::string& key) {
	_key = key;
}

const std::string& Channel::getKey() const {
	return _key;
}

bool Channel::hasKey() const {
	return !_key.empty();
}

void Channel::setUserLimit(int limit) {
	_userLimit = limit;
}

int Channel::getUserLimit() const {
	return _userLimit;
}

bool Channel::isFull() const {
	return _userLimit > 0 && _members.size() >= static_cast<size_t>(_userLimit);
}

void Channel::inviteClient(int fd) {
	_invited.insert(fd);
}

bool Channel::isInvited(int fd) const {
	return _invited.count(fd) > 0;
}

bool Channel::isValidName(const std::string& name)
{
	if (name.length() < 2 || name.length() > 50)
		return false;

	if (name[0] != '#' && name[0] != '&' && name[0] != '+' && name[0] != '!')
		return false;

	for (size_t i = 1; i < name.length(); ++i)
	{
		char c = name[i];
		if (c == ' ' || c == ',' || c == 7)
			return false;
	}
	return true;
}


const std::string& Channel::getName() const {
	return _name;
}

int Channel::countOperators() const
{
	int count = 0;
	for (std::map<int, bool>::const_iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (it->second)
			++count;
	}
	return count;
}
