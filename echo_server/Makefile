CC = gcc
flag = -lpthread -levent -o

all:server client

server:server.c
	$(CC) $< $(flag) $@

client:client.c
	$(CC) $< $(flag) $@

clean:
	rm -rf server client