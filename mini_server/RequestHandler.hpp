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


extern std::map<std::string, std::string> mimeTypesMap_G;

typedef struct parser
{
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
} parser;


class RequestHandler
{
public:
	void parseRequest();
	void parseHeaders();
	void parse_first_line();
	void receiveData(int clientSocket);
	parser& getRequest();
private:
	std::string _buffer;
	parser request;
};

// utility functions
std::string intToString(int value);
int stringToInt(const std::string &str);
std::string deleteSpaces(std::string const &str);
std::string get_file_extension(const std::string &file_path);
std::string get_file_name(const std::string &file_path);
bool isDirectory(const std::string &path);
std::vector<std::string> split(const std::string &str, char delimiter);

// debugging functions
void print_map(std::map<std::string, std::string> const &m);

void initializeMimeTypesMap();

void printServerVec(std::vector<ServerBlock> &_serverVec);