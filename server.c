#include <stdio.h>
#include <stdio.h>
#include "debug.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h>

typedef void (*a)(void);
typedef struct abc{
	a p;
	void *ptr;
	int sockfd;
}ABC;
void sock_handler(void)
{
	printf("my own socket handler\n");
}

int main()
{
	ABC ll;
	ll.p = sock_handler;
	ll.ptr = NULL;

	ll.p();
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == listenfd)
		errsys("socket");

	struct sockaddr_in myaddr = {0};
	struct sockaddr_in clientaddr = {0};
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(8888);
	myaddr.sin_addr.s_addr = inet_addr("0.0.0.0");//INADDR_ANY
	int len = sizeof myaddr;

	if(-1 == bind(listenfd, (struct sockaddr*)&myaddr, len))
		errsys("bind");

	if(-1 == listen(listenfd, 10))
		errsys("listen");

	int epoll_fd = epoll_create(1024);
	if(-1 == epoll_fd)
		errsys("epoll");

	struct epoll_event event = {0};
	event.events = EPOLLIN;
	ll.sockfd = listenfd;
	event.data.ptr = (void*)&ll;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &event);

#define BUFSIZE 100
#define MAXNFD  1024 
	
	struct epoll_event revents[MAXNFD] = {0};
	int nready;
	char buf[MAXNFD][BUFSIZE] = {0};
	while(1)
	{
		if(-1 == (nready = epoll_wait(epoll_fd, revents, MAXNFD, -1)) )
			errsys("poll");
		
		
		int i = 0;
		for(;i<nready; i++)
		{
			if(revents[i].events & EPOLLIN)
			{
//		
			
				ABC *cc = (ABC *)revents[i].data.ptr;
	
				if(cc->sockfd == listenfd)
				{
					int sockfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
					if(-1 == sockfd)
						errsys("accept");
					debug("incoming: %s\n", inet_ntoa( clientaddr.sin_addr) );
					
					struct epoll_event event = {0};
					event.events = EPOLLIN;
					ABC ll;
					ll.p = sock_handler;
					ll.sockfd = sockfd;
					event.data.ptr = (void*)&ll;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);
				}
				else
				{
					int ret = read(cc->sockfd, buf[cc->sockfd], sizeof buf[0]);
					if(0 == ret)
					{
			//			close(revents[i].data.fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cc->sockfd, &revents[i]);
						
					}
					
					revents[i].events = EPOLLOUT;
//					ABC *ll = (ABC *)revents[i].data.ptr;
					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, cc->sockfd, &revents[i]);
				}

			}
			else if(revents[i].events & EPOLLOUT)
			{
				ABC *ll = (ABC*)(revents[i].data.ptr);
				ll->p();
				int ret = write(ll->sockfd, buf[ll->sockfd], sizeof buf[0]);
				printf("ret %d: %d\n", ll->sockfd, ret);
				revents[i].events = EPOLLIN;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ll->sockfd, &revents[i]);
			}
		}
	}

	close(listenfd);
}
