#pragma once 

#include <sys/socket.h>
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

#define MAX_EVENTS 10

extern std::map<std::string, std::string> mimeTypesMap_G;

typedef struct parser
{
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
} parser;

typedef struct Response
{
	std::string status;
	std::string contentType;
	std::string contentLength;
	std::string body;
} Response;

class Server
{
public:
	Server();
	~Server();
	void createSocket();
	void bindSocket();
	void listenSocket();
	void acceptConnection();
private:
	int initializeEpoll();
	void handleEvents(int epoll_fd, std::vector<struct epoll_event>& events);
	void acceptClientConnections(int epoll_fd);
	void handleClientData(int client_fd);
	void read_file(std::string const &path);
	void receiveData();
	void parseRequest();
	void sendResponse();
	void notfound();
	void parseHeaders();
	void parse_first_line();
	int serverSocket;
	int clientSocket;
	std::string buffer;
	parser request;
	Response response;
};


//utility functions for parsing
std::string intToString(int value);
std::string deleteSpaces(std::string const &str);
std::string get_file_extension(const std::string& file_path);
std::string get_file_name(const std::string& file_path);
void initializeMimeTypesMap();
int make_socket_non_blocking(int sockfd);



//debuging and printing
void print_map(std::map<std::string, std::string> const &m);