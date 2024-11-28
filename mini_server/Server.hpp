#pragma once 
#include "Epoll.hpp"
// #define MAX_EVENTS 10
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
	int create_and_configure_socket();
	void bindNamesWithPorts(std::vector<std::string>& serverName, std::vector<int> serverPort);
};


void signalHandler(int signal);
