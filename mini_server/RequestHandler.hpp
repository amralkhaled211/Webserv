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
// #include "config_parser/Parser.hpp"
#include "Client.hpp"

class Client;

#define RESET_COLOR "\033[0m"
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR "\033[36m"
#define WHITE_COLOR "\033[37m"

#define	READ_CHUNK_SIZE	64000

extern std::map<std::string, std::string> mimeTypesMap_G;

class RequestHandler
{
	public:
		RequestHandler();
		void receiveData(int clientSocket, std::vector<Client> &_clients);
		void findClient(int clientSocket, std::vector<Client> &_clients);
		Client &findAllRecieved(std::vector<Client> clients);
	
	private:
		std::string _buffer;
		bool readytoSend;
};




// utility functions
std::string intToString(int value);
int stringToInt(const std::string &str);
std::string deleteSpaces(std::string const &str);
std::string get_file_extension(const std::string &file_path);
std::string get_file_name(const std::string &file_path);
bool isDirectory(const std::string &path);
std::vector<std::string> split(const std::string &str, char delimiter);
std::string sizeTToHexString(size_t value);
std::string decodeURIComponent(const std::string& component, bool isQueryString = false);
std::vector<std::string>	possibleRequestedLoc(std::string uri);

// debugging functions
void print_map(std::map<std::string, std::string> const &m);

void initializeMimeTypesMap();

void printServerVec(std::vector<ServerBlock> &_serverVec);