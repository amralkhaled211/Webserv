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
	private :
		static int _epollFD;
		static RequestHandler requestHandle;
	public :
		static void acceptConnection(const std::vector<int>& serverSockets);
		static void init_epoll(const std::vector<int>& serverSockets);
		static void handleEpollEvents(const std::vector<int>& serverSockets);
		static void handleConnection(int server_fd);
		static void handleData(int client_fd);
		static void close_epoll();
};
int make_socket_non_blocking(int sockfd);