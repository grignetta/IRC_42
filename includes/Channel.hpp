#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>
#include <map>

class Channel
{
	public:
		Channel(const std::string& name);
		~Channel();

		void addClient(int fd, bool isOperator = false);
		void removeClient(int fd);
		bool hasClient(int fd) const;
		
		bool isOperator(int fd) const;
		void promoteOperator(int fd);
		void demoteOperator(int fd);
		
		const std::map<int, bool>& getMembers() const;// is it nessary?

		void setTopic(const std::string& topic);
		const std::string& getTopic() const;
		
		void setTopicRestricted(bool value);
		bool isTopicRestricted() const;

		void setInviteOnly(bool value);
		bool isInviteOnly() const;
		void usedInvite(int fd);

		void setKey(const std::string& key);
		const std::string& getKey() const;
		bool hasKey() const;

		void setUserLimit(int limit);
		int getUserLimit() const;

		bool isFull() const;

		void inviteClient(int fd);
		bool isInvited(int fd) const;

		const std::string& getName() const;
	private:
		std::string _name;
		std::string _topic;
		std::map<int, bool> _members;// bool for ops
		bool _inviteOnly __attribute__((unused));
		bool _topicRestricted __attribute__((unused));
		std::string _key;
		int _userLimit __attribute__((unused));
		std::set<int> _invited;       // For +i mode

	// Methods: addClient(), removeClient(), isOperator(), etc.
};

#endif