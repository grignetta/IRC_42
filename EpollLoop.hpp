#ifndef EPOLLLOOP_HPP
#define EPOLLLOOP_HPP

#include "IEventLoop.hpp"
#include <sys/epoll.h>

#define MAX_EVENTS 256

class EpollEventLoop : public IEventLoop
{
	public:
		void setup(int serverFd);
		int wait();
		int getReadyFd(int index) const;
	private:
		int _epollFd;
		epoll_event _events[MAX_EVENTS];//array of structs
};

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

