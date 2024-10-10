#pragma once 

#include "Epoll.hpp"
#define MAX_EVENTS 10




class Server
{
public:
	Server(std::vector<ServerBlock>& _serverVec);
	~Server();
	void run();
private:
	// variables
	int serverSocket;
	int clientSocket;
	std::vector<ServerBlock>& _servers;
	std::vector<int> _serverSockets;
	void createSocket();
	void accept();

};


void signalHandler(int signal);
