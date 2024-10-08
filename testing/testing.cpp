#include "server.hpp"


//AF_INET: This specifies the address family. AF_INET indicates that the socket will use the IPv4 protocol.

/* SOCK_STREAM: This specifies the socket type. SOCK_STREAM indicates that the socket will be a stream socket,
which provides reliable, two-way, connection-based byte streams. This is typically used for TCP connections. */

/* 0: This specifies the protocol to be used by the socket. A value of 0 means that the default protocol for the
 specified address family and socket type will be used. For AF_INET and SOCK_STREAM, this is typically TCP */

// int main()
// {
// 	//we create here a socket. The function returns a fd
// 	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// 	if (serverSocket < 0)
// 	{
// 		std::cerr << "Socket creation failed" << std::endl;
// 		return -1;
// 	}

// 	//We then define the server address using the following set of statements
// 	sockaddr_in serverAddress; // sockaddr_in: It is the data type that is used to store the address of the socket.
// 	serverAddress.sin_family = AF_INET;
// 	serverAddress.sin_port = htons(8080); //Without this conversion, the port number might be interpreted incorrectly by other machines on the network, leading to communication issues.
// 	serverAddress.sin_addr.s_addr = INADDR_ANY;// INADDR_ANY: It is used when we don’t want to bind our socket to any particular IP and instead make it listen to all the available IPs.

// 	//binds the serverSocket to the address and port specified in serverAddress. This is a necessary step before the socket can listen for incoming connection
// 	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
// 	{
// 		std::cerr << "Binding failed" << std::endl;
// 		return -1;
// 	}

// 	//listening to request connection 
// 	if (listen(serverSocket, 5) < 0)// defines the maximum number of pending connections
// 	{
// 		std::cerr << "Listening failed" << std::endl;
// 		return -1;
// 	}


// 	//accept the connection request that is recieved on the socket the application was listening to.
// 	//waits for an incoming connection on serverSocket, and if a connection is successfully established, it assigns the new socket descriptor to clientSocket.
// 	int clientSocket = accept(serverSocket, nullptr, nullptr);
// 	if (clientSocket < 0)
// 	{
// 		std::cerr << "Accepting failed" << std::endl;
// 		return -1;
// 	}

// 	char buffer[1024] = {0};
// 	recv(clientSocket, buffer, sizeof(buffer), 0);/// i need to parse the request here
// 	std::cout << "Message from client: " << buffer << std::endl;


// 	const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nHello";// i need to parse the response here and make it dynamic
//     send(clientSocket, response, strlen(response), 0);


// 	close(serverSocket);
// 	close(clientSocket);
// }






















// #include <iostream>
// #include <cstring>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <unistd.h>

// int main() {
//     const char* hostname = "alkhaled.com";
//     struct hostent* host_info = gethostbyname(hostname);
//     if (host_info == nullptr) {
//         std::cerr << "Error: Could not resolve hostname." << std::endl;
//         return 1;
//     }

//     struct in_addr* address = (struct in_addr*)host_info->h_addr;
//     std::cout << "Resolved IP address: " << inet_ntoa(*address) << std::endl;

//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) {
//         std::cerr << "Error: Could not create socket." << std::endl;
//         return 1;
//     }

//     struct sockaddr_in server_addr;
//     std::memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(80); // Example port
//     std::memcpy(&server_addr.sin_addr, address, sizeof(struct in_addr));

//     if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
//         std::cerr << "Error: Could not bind socket." << std::endl;
//         close(sockfd);
//         return 1;
//     }

//     std::cout << "Socket bound to " << hostname << std::endl;

//     // Continue with the rest of your server code...

//     close(sockfd);
//     return 0;
// }