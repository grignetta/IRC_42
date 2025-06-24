#include "PollLoop.hpp"

void PollEventLoop::setup(int serverFd)
{
	pollfd pfd;
	pfd.fd = serverFd;
	pfd.events = POLLIN;
	_pollFds.push_back(pfd);
}

int PollEventLoop::wait()
{
	return poll(&_pollFds[0], _pollFds.size(), -1);
}

int PollEventLoop::getReadyFd(int index) const
{
	return _pollFds[index].fd;
}