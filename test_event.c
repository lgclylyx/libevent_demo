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

void callback(int fd,short event, void *){
	char recvbuff[1];
	recv(fd,recvbuff,1,0);
	printf("file description %d occured event (%x).\n",fd,event);
}

void* thread(void* arg){
	while(1){
		int fd = *((int*)arg);
		send(fd, "a", 1, 0);
		sleep(5);
	}
	return NULL;
}



int main(){
	struct event_base* base = event_base_new();
	struct event ev;
	int fd[2];
	pthread_t ppid;

	printf("socket pairs.\n");
	assert(evutil_socketpair(AF_UNIX,SOCK_STREAM,0,fd) != -1);

	printf("event set.\n");
	event_set(&ev,fd[0],EV_READ | EV_PERSIST,callback,NULL);

	printf("thread.\n");
	pthread_create(&ppid,NULL,thread,&fd[1]);

	printf("base set.\n");
	assert(event_base_set(base,&ev) != -1);

	printf("event add.\n");
	assert(event_add(&ev,NULL) != -1);

	event_base_dispatch(base);
	event_base_free(base);
}
