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

/*
 * STD_IN输入事件的回调函数，读取键盘的输入并发送给服务器。
 * @param arg 与服务器的连接的struct bufferevent
 * */
void callback_keyboard(evutil_socket_t fd, short event, void * arg){
	struct bufferevent* bev = (struct bufferevent*)arg;
	char recvbuff[1024];
	int nread = read(0,recvbuff,sizeof(recvbuff));
	if(nread < 0){
		printf("read error.\n");
		return;
	}
	bufferevent_write(bev,recvbuff,nread);
}

void callback_read(struct bufferevent *bev, void * arg){
	char recvbuff[1024];
	size_t len = bufferevent_read(bev, recvbuff, sizeof(recvbuff) - 1);
	recvbuff[len] = '\0';
	printf("recv %lu bytes: %s",len,recvbuff);
}

void callback_error(struct bufferevent *bev, short what, void * arg){
	if(what & BEV_EVENT_EOF)
		printf("connection closed.\n");
	else if(what & BEV_EVENT_ERROR)
		printf("occurred error.\n");
	else if(what & BEV_EVENT_CONNECTED){
		printf("connection seccesses.\n");
		return;
	}
	//在bufferevent_socket_new时设置了BEV_OPT_CLOSE_ON_FREE，则连接会自动关闭
	bufferevent_free(bev);
	//所有监听的事件全部移除后，就会关闭进程
	event_free((struct event*)arg);
}

int main(int argc, char** argv) {
	if (argc < 3)
		printf("error usage;\n ./client 127.0.0.1 9999\n");

	struct sockaddr_in remote_addr;
	bzero(&remote_addr, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(argv[1]);
	remote_addr.sin_port = htons((unsigned short) atoi(argv[2]));

	struct event_base* base = event_base_new();

	struct bufferevent* bev = bufferevent_socket_new(base,-1,BEV_OPT_CLOSE_ON_FREE);

	//本地键盘输入事件
	struct event* ev = event_new(base,0,EV_READ|EV_PERSIST,callback_keyboard,bev);
	event_add(ev,NULL);

	bufferevent_setcb(bev,callback_read,NULL,callback_error,ev);
	bufferevent_enable(bev,EV_READ|EV_PERSIST);

	if(bufferevent_socket_connect(bev,(struct sockaddr*)&remote_addr,sizeof(remote_addr)) == -1){
		printf("connection failure.\n");
		exit(0);
	}

	event_base_dispatch(base);
}
