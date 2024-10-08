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

class Epoll
{
	public :
	Epoll( const std::vector<int>& serverSockets);
	~Epoll();

	private :
	int _epollFD;
	const std::vector<int>& serverSockets;
	RequestHandler requestHandle;
	// std::vector<struct epoll_event>  _events(MAX_EVENTS);
	void init_epoll();
	void handleEpollEvents( std::vector<struct epoll_event>& _events);
	void handleConnection(int server_fd);
	void acceptConnection();
	void handleData(int client_fd);



};

int make_socket_non_blocking(int sockfd);