#ifndef POLLLOOP_HPP
# define POLLLOOP_HPP

# ifdef __APPLE__
# include "IEventLoop.hpp"
# include <vector>
# include <poll.h>

class PollEventLoop : public IEventLoop
{
	public:
		void setup(int serverFd);
		//int wait();
		std::vector<int> wait();
		void addFd(int clientFd);
		void removeFd(int clientFd);
	private:
		std::vector<pollfd> _pollFds;

};

# endif
#endif
