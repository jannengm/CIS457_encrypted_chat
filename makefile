#Makefile

make: server client clean

server: chat_server.o tcp_chat.o client_list.o encrypt.o
	gcc -Wall chat_server.o tcp_chat.o encrypt.o client_list.o -lcrypto -o server

client: chat_client.o tcp_chat.o encrypt.o
	gcc -Wall chat_client.o tcp_chat.o encrypt.o -pthread -lcrypto -o client

chat_server.o:
	gcc -Wall -c chat_server.c client_list.h tcp_chat.h encrypt.h

chat_client.o:
	gcc -Wall -c chat_client.c tcp_chat.h encrypt.h

tcp_chat.o:
	gcc -Wall -c tcp_chat.c tcp_chat.h

client_list.o:
	gcc -Wall -c client_list.c client_list.h tcp_chat.h

encrypt.o:
	gcc -Wall -c encrypt.c encrypt.h

clean:
	rm *.o
	rm *.gch
