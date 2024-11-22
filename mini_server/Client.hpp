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
#include <csignal>
#include <sys/stat.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <iomanip>
#include "config_parser/Parser.hpp"


typedef struct parser
{
	std::string method;
	std::string path;
	std::string version;
	std::string queryString;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string fileName;

} parser;


class Client
{
	public:
		Client();
		//~Client();
		void parse_first_line();
		void parseHeaders(std::string &Buffer);
		bool parse_body(std::string &body);
		bool HandlChunk();
		bool parseHeadersAndBody();
		void allRecieved();
		void setBuffer(const std::string& buffer);
		int getClientFD() { return _clientFD; }
		void setClientFD(int clientFD) { _clientFD = clientFD; }
		void setAllRecieved(bool allRecieved) { isAllRecieved = allRecieved; }
		bool getIsAllRecieved() { return isAllRecieved; }
		void parseQueryString();

		parser &getRequest();

		void setResponseBuffer(std::string resBuffer);
		std::string& getResponseBuffer() { return _responseBuffer; }
	
	private:
	std::string _buffer;			// for storing and processing request header & body; more accurate name: _requestBuffer
	std::string _responseBuffer;	// for storing and processing response header & body
	int _clientFD;
	parser request;
	std::string _boundary;
	bool _isChunked;
	size_t _targetBytes;
	size_t _bytesRead;
	bool isAllRecieved;
};


size_t stringToSizeT(const std::string& str);
