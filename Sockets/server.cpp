#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

//AF_INET: This specifies the address family. AF_INET indicates that the socket will use the IPv4 protocol.

/* SOCK_STREAM: This specifies the socket type. SOCK_STREAM indicates that the socket will be a stream socket,
which provides reliable, two-way, connection-based byte streams. This is typically used for TCP connections. */

/* 0: This specifies the protocol to be used by the socket. A value of 0 means that the default protocol for the
 specified address family and socket type will be used. For AF_INET and SOCK_STREAM, this is typically TCP */

int main()
{
	//we create here a socket. The function returns a fd
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);	

	//We then define the server address using the following set of statements
	sockaddr_in serverAddress; // sockaddr_in: It is the data type that is used to store the address of the socket.
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080); //Without this conversion, the port number might be interpreted incorrectly by other machines on the network, leading to communication issues.
	serverAddress.sin_addr.s_addr = INADDR_ANY;// INADDR_ANY: It is used when we don’t want to bind our socket to any particular IP and instead make it listen to all the available IPs.

	//binds the serverSocket to the address and port specified in serverAddress. This is a necessary step before the socket can listen for incoming connection
	bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	//listening to request connection 
	listen(serverSocket, 5);// defines the maximum number of pending connections


	//accept the connection request that is recieved on the socket the application was listening to.
	//waits for an incoming connection on serverSocket, and if a connection is successfully established, it assigns the new socket descriptor to clientSocket.
	int clientSocket = accept(serverSocket, nullptr, nullptr);

	char buffer[1024] = {0};
	recv(clientSocket, buffer, sizeof(buffer), 0);
	std::cout << "Message from client: " << buffer << std::endl;


	close(serverSocket);

}