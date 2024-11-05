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
#include "config_parser/Parser.hpp"

#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR "\033[36m"
#define WHITE_COLOR "\033[37m"

extern std::map<std::string, std::string> mimeTypesMap_G;

typedef struct parser
{
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string fileName;
} parser;

class RequestHandler
{
public:
	RequestHandler();
	bool parseRequest();
	void parse_first_line();
	bool parseHeadersAndBody();
	void parseHeaders(std::string &Buffer);
	bool parse_body(std::string &body);
	void receiveData(int clientSocket);
	bool HandlChunk();
	parser &getRequest();

private:
	std::string _buffer;
	parser request;
	bool _isChunked;
	size_t _bytesRead;
	size_t _targetBytes;
	std::string _boundary;
};

// utility functions
std::string intToString(int value);
int stringToInt(const std::string &str);
size_t stringToSizeT(const std::string& str);
std::string deleteSpaces(std::string const &str);
std::string get_file_extension(const std::string &file_path);
std::string get_file_name(const std::string &file_path);
bool isDirectory(const std::string &path);
std::vector<std::string> split(const std::string &str, char delimiter);

// debugging functions
void print_map(std::map<std::string, std::string> const &m);

void initializeMimeTypesMap();

void printServerVec(std::vector<ServerBlock> &_serverVec);