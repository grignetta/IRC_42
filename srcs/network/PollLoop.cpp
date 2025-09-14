#include "PollLoop.hpp"

# ifdef __APPLE__
void PollEventLoop::setup(int serverFd)
{
	pollfd pfd;
	pfd.fd = serverFd;
	pfd.events = POLLIN;
	_pollFds.push_back(pfd);
}

/*int PollEventLoop::wait()
{
	return poll(&_pollFds[0], _pollFds.size(), -1);
}*/

std::vector<int> PollEventLoop::wait() {
	int n = ::poll(_pollFds.data(), _pollFds.size(), -1);
	if (n < 0)
		throw std::runtime_error(std::string("poll() failed: ") + strerror(errno));

	std::vector<int> readyFds;
	for (std::vector<pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->revents & POLLIN)
			readyFds.push_back(it->fd);
	}
	return readyFds;
}

void PollEventLoop::addFd(int clientFd)
{
	pollfd pfd;
	pfd.fd     = clientFd;
	pfd.events = POLLIN;
	_pollFds.push_back(pfd);
}

void PollEventLoop::removeFd(int clientFd)
{
	for (std::vector<pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == clientFd) {
			_pollFds.erase(it);
			break;
		}
	}
}

#endif