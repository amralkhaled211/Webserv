#pragma once 

#include <sys/socket.h>
#include <algorithm>
#include <fstream>
#include <string>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <map>
#include <istream>
#include <cctype>
#include <netdb.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <csignal>
#include <cstdlib>
#include "RequestHandler.hpp"
#include "config_parser/Parser.hpp"

#define MAX_EVENTS 10

extern bool serverRunning ;

class Server
{
public:
	Server(std::vector<ServerBlock>& _serverVec);
	~Server();
	void run();
private:
	// variables
	int serverSocket;
	//int serverSocket2;
	int clientSocket;
	int epoll_fd;

	// objects
	RequestHandler requestHandle;
	std::vector<ServerBlock>& _servers;

	// functions
	void createSocket();
	void bindSocket();
	void listenSocket();
	void acceptConnection();

	//testing
	// void createSocket2();
	// void bindSocket2();
	// void listenSocket2();
};

int make_socket_non_blocking(int sockfd);
void signalHandler(int signal);
