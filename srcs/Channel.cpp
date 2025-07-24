#include "Channel.hpp"

Channel::Channel(const std::string& name)
	: _name(name), _userLimit(0), _inviteOnly(false), _topicRestricted(false) {}

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

bool Channel::isOperator(int fd) const
{
	std::map<int, bool>::const_iterator it = _members.find(fd);
	return it != _members.end() && it->second;
}

void Channel::promoteOperator(int fd)
{
	if (hasClient(fd))//why check it?
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

void Channel::usedInvite(int fd)
{
	_invited.erase(fd);
}

const std::string& Channel::getName() const {
	return _name;
}

