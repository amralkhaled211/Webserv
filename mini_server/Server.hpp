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
	std::vector<ServerBlock>& _servers;
	std::vector<int> _serverSockets;
	void createSocket();
	void bindNamesWithPorts(std::vector<std::string>& serverName, std::vector<int> serverPort, int serverSocket);
	void accept();

};


void signalHandler(int signal);
