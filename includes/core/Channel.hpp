#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>
#include <map>

class Channel
{
	public:
		Channel();
		Channel(const std::string& name);
		~Channel();

		void							addClient(int fd, bool isOperator);
		void							removeClient(int fd);
		
		void							promoteOperator(int fd);
		void							demoteOperator(int fd);
		
		const std::map<int, bool>&		getMembers() const;
		
		void							setInviteOnly(bool value);
		bool							isInviteOnly() const;
		
		/// getters ////
		int								getClientCount() const;
		const std::string& 				getTopic() const;
		int								getUserLimit() const;
		const std::string&				getKey() const;
		
		/// setters ////
		void 							setTopicRestricted(bool value);
		void 							setTopic(const std::string& topic);
		void							setUserLimit(int limit);
		void							setKey(const std::string& key);
		
		/// bools ////
		bool							hasClient(int fd) const;
		bool							isOperator(int fd) const;
		bool 							isTopicRestricted() const;
		bool							hasKey() const;
		bool							isFull() const;
		bool							isInvited(int fd) const;

		void inviteClient(int fd);
		int countOperators() const;

		static bool isValidName(const std::string& name);
		const std::string& getName() const;

	private:
		std::string _name;
		std::string _topic;
		std::map<int, bool> _members;// bool for ops
		bool _inviteOnly;
		bool _topicRestricted;
		std::string _key;
		int _userLimit;
		std::set<int> _invited;
};

#endif