all: client server

client: client.c utils.h
	gcc -Wall -g client.c -o client -lreadline

server: server.c
	gcc -Wall -g server.c -o server -pthread

clean:
	rm client server *~