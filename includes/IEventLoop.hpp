#ifndef IEVENTLOOP_HPP
#define IEVENTLOOP_HPP

# include "Exception.hpp"
# include <vector>

class IEventLoop
{
	public:
		virtual void setup(int serverFd) = 0;
		//virtual int wait() = 0;
		virtual std::vector<int> wait() = 0;
		virtual void addFd(int clientFd) = 0;
		virtual ~IEventLoop() {}
};

#endif