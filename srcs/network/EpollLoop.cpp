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

std::vector<int> EpollEventLoop::wait()
{
	int n = epoll_wait(_epollFd, _events, MAX_EVENTS, -1);//returns the number of file descriptors ready for reading
	if (n < 0)
		throw std::runtime_error(std::string("epoll_wait() failed: ") + strerror(errno));

	std::vector<int> readyFds;
	for (int i = 0; i < n; ++i) {
		readyFds.push_back(_events[i].data.fd);
	}
	return readyFds;
}

void EpollEventLoop::addFd(int clientFd) {
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = clientFd;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientFd, &ev) == -1)
        throw SocketException(std::string("epoll_ctl ADD client failed: ") 
                              + strerror(errno));
}

void EpollEventLoop::removeFd(int clientFd) {
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, NULL) == -1)
        throw SocketException(std::string("epoll_ctl DEL client failed: ") 
                              + strerror(errno));
}

#endif