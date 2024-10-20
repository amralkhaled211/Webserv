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

typedef struct Response
{
	std::string status;
	std::string contentType;
	std::string contentLength;
	std::string body;
} Response;

class RequestHandler
{
	public:
		//RequestHandler();
		//~RequestHandler();
		bool read_file(std::string const &path);
		bool findIndexFile(const std::vector<std::string>& files, std::string& root);
		void receiveData(int clientSocket);
		void parseRequest();
		void sendResponse(int clientSocket, std::vector<ServerBlock>& servers);
		ServerBlock findServerBlock(std::vector<ServerBlock>& servers);
		LocationBlock findLocationBlock(std::vector<LocationBlock>& locations);
		void notfound();
		void parseHeaders();
		void parse_first_line();
		//void configResponse(ServerBlock &server);
		template <typename T>
        bool findInVector(const std::vector<T>& vec, const T& target)
		{
            return std::find(vec.begin(), vec.end(), target) != vec.end();
        }
	private:
	bool _isDir;
	parser request;
	Response response;
	std::string buffer;
};

//utility functions
std::string intToString(int value);
int stringToInt(const std::string& str);
std::string deleteSpaces(std::string const &str);
std::string get_file_extension(const std::string& file_path);
std::string get_file_name(const std::string& file_path);

//debugging functions
void print_map(std::map<std::string, std::string> const &m);

void initializeMimeTypesMap();

void	printServerVec(std::vector<ServerBlock>& _serverVec);