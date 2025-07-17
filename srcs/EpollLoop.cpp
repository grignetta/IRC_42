# ifdef __linux__

#include "EpollLoop.hpp"
#include <stdexcept>
#include <unistd.h>

EpollEventLoop::EpollEventLoop()
{
	_epollFd = -1;
}

EpollEventLoop::~EpollEventLoop()
{
	if (_epollFd != -1)
		close(_epollFd);
}

void EpollEventLoop::setup(int serverFd)
{
	_epollFd = epoll_create1(0);// 0 means no special flags
	if (_epollFd == -1)
		throw SocketException(std::string("epoll_create1 failed: ") + strerror(errno));
	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = serverFd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1)
		throw SocketException(std::string("epoll_ctl failed: ") + strerror(errno));
}

int EpollEventLoop::wait()
{
	return epoll_wait(_epollFd, _events, MAX_EVENTS, -1);//returns the number of file descriptors ready for reading
}

int EpollEventLoop::getReadyFd(int index) const
{
	return _events[index].data.fd;
}

#endif