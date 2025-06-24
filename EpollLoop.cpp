#include "EpollLoop.hpp"
#include <stdexcept>
#include <unistd.h>

void EpollEventLoop::setup(int serverFd)
{
    _epollFd = epoll_create1(0);
    if (_epollFd == -1)
        throw std::runtime_error("epoll_create1 failed");

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = serverFd;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1)
        throw std::runtime_error("epoll_ctl failed");
}

int EpollEventLoop::wait()
{
    return epoll_wait(_epollFd, _events, 64, -1);
}

int EpollEventLoop::getReadyFd(int index) const
{
    return _events[index].data.fd;
}
