//libevent
#include "event.h"
//c lib
#include <stdio.h>
#include <assert.h>
//system call
#include <unistd.h> 
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

void callback_read(struct bufferevent *bev, void * arg){
	char recvbuff[1];
	bufferevent_read(bev,recvbuff,1);
	printf("file description %d occured event (%x).\n",bev->ev_read.ev_fd,bev->ev_read.ev_events);
}
void callback_error(struct bufferevent *bev, short what, void * arg){
	printf("file description %d occured error (%x).\n",bev->ev_read.ev_fd,what);
}

void* thread(void* arg){
	while(1){
		int fd = *((int*)arg);
		send(fd, "a", 1, 0);
		sleep(5);
	}
	return NULL;
}


//use bufferevent
int main(){
	struct event_base* base = event_base_new();
	struct bufferevent* bev;
	int fd[2];
	pthread_t ppid;

	printf("socket pairs.\n");
	assert(evutil_socketpair(AF_UNIX,SOCK_STREAM,0,fd) != -1);

	printf("bufferevent new.\n");
	bev = bufferevent_socket_new(base,fd[0],BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev,callback_read,NULL,callback_error,NULL);
	/*
	Reading needs to be explicitly enabled
	because otherwise no data will be available.
	*/
	bufferevent_enable(bev,EV_READ|EV_PERSIST);

	printf("thread.\n");
	pthread_create(&ppid,NULL,thread,&fd[1]);

	event_base_dispatch(base);
	bufferevent_free(bev);
	event_base_free(base);
}