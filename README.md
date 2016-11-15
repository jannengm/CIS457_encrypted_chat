# TCP Encrypted Chat Program
## CIS 457 - F16 - Project 3

###Overview
The purpose of this program was to explore the functionality of symmetric and asymmetric encryption protocols defined in the OpenSSL libraries by creating an encrypted TCP chat server and client. The server accepts connections to multiple simultaneous clients. The clients each randomly generate a symmetric key which is then communicated to the server using asymmetric encryption with the server’s public RSA key. The server then decrypts messages that it receives from the clients, re-encrypts them with the appropriate symmetric key for the intended destination client, then forwards the message. The functionality of the server and client are characterized as follows:

####Server
- Sets up a socket on a user specified port
- Uses select to read in messages from multiple clients
- Maintains a linked list of connected clients
- Receives and decrypts messages from clients
- Processes administrative commands sent from clients
- Encrypts and forwards messages to their intended destinations

####Client
- Connects to a server at a specified port and IPv4 address
- Creates and sends a random symmetric key to the server
- Encrypts and sends user messages to the server
- Receives and decrypts messages from the server

    The server and client were implemented in C in the files chat_server.c and chat_client.c, respectively. Both files make use of additional functions and constants defined in tcp_chat.h, client_list.h, and encrypt.h as well as the associated source code files tcp_chat.c, client_list.c, and encrypt.c. A makefile is provided for easy compilation using make. The syntax for the execution of the server and clients, respectively, is:
    
./server [port #]
./client [port#] [IPv4 address]

##Server
###Setting up the Server Socket
    The server sets up a new master socket (sockfd) to listen for new client connections on the port supplied as a command line argument. This master socket is added to an fd_set (active_set).


###Reading in from Clients
    The server loops continuously and uses select to read from any file descriptors in the fd_set with pending input. If the pending input is not on the master socket, then the input is handed off to the handle_client function. If, however, the pending input is on the master socket, then a new client must have connected. In this case, a new socket file descriptor is created for this client, which is then added to the fd_set. The server then waits to receive a plain text initialization vector and an encrypted symmetric key from the client. The server decrypts the key with the server’s own private key, and adds the initialization vector, the symmetric key, and the file descriptor to a new client_t struct. The server also assigns an ID number to the client. This new client_t struct is then added to a linked list of client_t structs.


###Maintaining a Linked List of Clients
    The server keeps track of all active client connections by maintaining a linked list of clients. The linked list itself is defined as a client_list_t struct with a head pointer and a tail pointer. Each node of the linked list is defined as a client_node_t struct with next and previous pointers, as well as client data. The client data is stored in a client_t struct which has a client ID number, a client file descriptor, and an initialization vector and symmetric key for that specific client. Struct definitions and associated functions for the linked list are defined in client_list.h and client_list.c.


###Receives Messages from Clients
    Once a client has been successfully connected and a symmetric key established, new input from that client is handled by the handle_client function. This function takes a pointer to a client_t struct to designate the sender. The server reads in a message from the file descriptor stored in the sender parameter, and then decrypts it with the initialization vector and symmetric key stored in the sender parameter.


###Processing Administrative Commands
    Once a message is decrypted, the function check_command is called to determine what, if any, administrative commands are contained in the message. This function is defined in tcp_chat.h and tcp_chat.c. Administrative commands begin with the ‘!’ character. Valid commands are “!exit”, “!kill [target]”, “!list”, and “!shutdown”. Exit performs an orderly shutdown of the client that sent the command. Kill performs an orderly shutdown of the client designated by the target. List sends a formatted list of all connected clients to the requesting client. Shutdown performs an orderly shutdown of all connected clients and then shuts down the server. If messages are not commands, they should have an “@[target ID]” prefix. The same function determines if this prefix is present, and if so, sets the value of a passed target parameter appropriately.


###Encrypting and Forwarding Messages
    Once the target of a message is determined, the server calls the send_to_target function defined in client_list.h and client_list.c which takes the message, cycles through the list of connected clients, and encrypts and sends the message to either the client with the matching ID, or to all clients except the sender if the target is designated to be BROADCAST.


##Client
###Connecting to the Server
    The client attempts to connect to the server at the port and IPv4 address specified as command line parameters. If these parameters are missing or invalid, it prints an error and exits.


##Creating a Symmetric Key
    Once a connection to the server is established, the client uses the create_symmetric_key function to randomly generate an initialization vector and a key. It then uses the send_symmetric_key function to encrypt the new key using the server’s public key, and then sends the initialization vector (as plain text) and the encrypted key to the server.


##Sending Messages to the Server
    The client loops continuously to get user input from stdin. If an “@[target ID]” tag is entered as a prefix, the client sets that target as the default, otherwise the default is to broadcast to all clients. To reset the target to be a broadcast, the “@all” prefix is entered. Once a message is read it, it is encrypted with the initialization vector and symmetric key and then sent to the server. If “!exit” is entered, the client sends the command to the server and then performs an orderly shutdown.


##Receiving Messages from the Server
    The client creates a separate thread to wait for messages from the server. Once a message is received, it is decrypted using the initialization vector and symmetric key. If “!exit” was received, the client performs an orderly shutdown. Otherwise, the decrypted message is printed to stdout.
