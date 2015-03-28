all: client server bossman

client: client.o utils.o
	gcc -Wall -g client.o utils.o -o client.out -lreadline

server: server.c
	gcc -Wall -g server.c -o server.out

client.o: client.c
	gcc -Wall -g client.c -c

bossman: server/server.c utils.o
	gcc -Wall -g -pthread utils.o server/server.c server/clientlist.c server/sessionlist.c -o server/server.out

utils.o: utils.h utils.c
	gcc -Wall -g utils.c -c

clean:
	rm client.out server/server.out *~