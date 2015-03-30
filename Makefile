all: client server

client: client.o utils.o
	gcc -Wall -g client.o utils.o -o client -lreadline

client.o: client.c
	gcc -Wall -g client.c -c

server: server/server.c utils.o
	gcc -Wall -g -pthread utils.o server/server.c server/clientlist.c server/sessionlist.c -o server/server

utils.o: utils.h utils.c
	gcc -Wall -g utils.c -c

clean:
	rm client server/server *.o *~
