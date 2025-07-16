#ifndef IEVENTLOOP_HPP
#define IEVENTLOOP_HPP

# include "Exception.hpp"

class IEventLoop
{
	public:
		virtual void setup(int serverFd) = 0;
		virtual int wait() = 0;
		virtual int getReadyFd(int index) const = 0;
		virtual ~IEventLoop() {}
};

#endif