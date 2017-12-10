//libevent
#include "event.h"
#include "event2/listener.h"
//system call
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
 #include <arpa/inet.h>
//c lib
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

void callback_read(struct bufferevent *bev, void * arg){
	char recvbuff[1024];
	size_t len = bufferevent_read(bev, recvbuff, sizeof(recvbuff) - 1);
	recvbuff[len] = '\0';
	printf("recv %lu bytes: %s",len,recvbuff);
	bufferevent_write(bev,recvbuff,len);
}

void callback_error(struct bufferevent *bev, short what, void * arg){
	if(what & BEV_EVENT_EOF)
		printf("connection closed.\n");
	else if(what & BEV_EVENT_ERROR)
		printf("occurred error.\n");
	//在bufferevent_socket_new时设置了BEV_OPT_CLOSE_ON_FREE，则连接会自动关闭
	bufferevent_free(bev);
}

/*
 * 处理新连接的回调函数，其为到来的连接创建一个bufferevent事件，并且
 * 设置回调函数并加入I/O复用机制中
 * @param listener
 * @param fd 新到来连接的描述符
 * @param addr 新到来连接的源地址
 * @param arg 传给该回调函数的参数，在evconnlistener_new_bind中设置
 * */
void callback_listener(struct evconnlistener * listener, evutil_socket_t fd,
		struct sockaddr * addr, int socklen, void * arg) {
	struct sockaddr_in* sin = (struct sockaddr_in*)addr;
	printf("accepted connection %d from ip %s port %d.\n",fd,inet_ntoa(sin->sin_addr),sin->sin_port);

	struct event_base* base = (struct event_base*)arg;
	//为新到来的连接创建一个bufferevent
	struct bufferevent* bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	//为其设置回调函数
	bufferevent_setcb(bev,callback_read,NULL,callback_error,NULL);
	//将其加入系统提供的I/O事件侦听中
	bufferevent_enable(bev,EV_READ|EV_PERSIST);
}

int main(int argc, char** argv) {
	if (argc < 3)
		printf("error usage;\n ./server 127.0.0.1 9999\n");

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons((unsigned short) atoi(argv[2]));

	struct event_base* base = event_base_new();
	/*
	 * evconnlistener_new_bind（）根据给定的地址，创建一个SOCKET描述符，并绑定监听，
	 * 加入libevent的事件监听中
	 * */
	struct evconnlistener* evlistener = evconnlistener_new_bind(base,
			callback_listener, base,
			LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 5,
			(struct sockaddr*) &addr, sizeof(addr));

	event_base_dispatch(base);

	evconnlistener_free(evlistener);
	event_base_free(base);
}


