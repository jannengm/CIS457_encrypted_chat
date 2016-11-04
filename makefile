#Makefile

make: server client clean

server: chat_server.o tcp_chat.o
	gcc -Wall chat_server.o tcp_chat.o -o server

client: chat_client.o tcp_chat.o
	gcc -Wall chat_client.o tcp_chat.o -pthread -o client

chat_server.o:
	gcc -Wall -c chat_server.c tcp_chat.h

chat_client.o:
	gcc -Wall -c chat_client.c tcp_chat.h

tcp_chat.o:
	gcc -Wall -c tcp_chat.c tcp_chat.h

clean:
	rm *.o
	rm *.gch