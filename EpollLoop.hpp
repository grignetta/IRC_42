#ifndef EPOLLLOOP_HPP
#define EPOLLLOOP_HPP

#include "IEventLoop.hpp"
#include <sys/epoll.h>

class EpollEventLoop : public IEventLoop
{
	public:
		void setup(int serverFd);
		int wait();
		int getReadyFd(int index) const;
	private:
		int _epollFd;
		epoll_event _events[64];//why  64 only?
};

#endif
