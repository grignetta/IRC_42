#ifndef EPOLLLOOP_HPP
# define EPOLLLOOP_HPP

# ifdef __linux__
# include "IEventLoop.hpp"

# include <sys/epoll.h>


//#include <sys/epoll.h>

# define MAX_EVENTS 256

class EpollEventLoop : public IEventLoop
{
	public:
		EpollEventLoop();
		~EpollEventLoop();
		void setup(int serverFd);
		std::vector<int> wait();
		void addFd(int clientFd);
		void removeFd(int clientFd);
	private:
		int _epollFd;
		epoll_event _events[MAX_EVENTS];//array of structs
};

# endif
#endif

// #include <sys/epoll.h>

//        struct epoll_event {
//            uint32_t      events;  /* Epoll events */
//            epoll_data_t  data;    /* User data variable */
//        };

//        union epoll_data {
//            void     *ptr;
//            int       fd;
//            uint32_t  u32;
//            uint64_t  u64;
//        };

//        typedef union epoll_data  epoll_data_t;

