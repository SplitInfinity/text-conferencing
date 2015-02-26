all: client server

client: client.c utils.h
	gcc -Wall -g client.c -o client.out -lreadline

server: server.c
	gcc -Wall -g server.c -o server.out

clean:
	rm client server *~