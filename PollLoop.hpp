#ifndef POLLLOOP_HPP
#define POLLLOOP_HPP

#include "IEventLoop.hpp"
#include <vector>
#include <poll.h>

class PollEventLoop : public IEventLoop
{
private:
	std::vector<pollfd> _pollFds;

public:
	void setup(int serverFd);
	int wait();
	int getReadyFd(int index) const;
};

#endif
