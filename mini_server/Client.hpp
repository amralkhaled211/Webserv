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
#include "RequestHandler.hpp"
#include "Response.hpp"
#include <ctime>
// #include "SendData.hpp"


struct Response;

typedef struct parser
{
	std::string method;
	std::string path;
	std::string version;
	std::string queryString;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string fileName;
	int statusError ;
} parser; // better name: requestParser

#define	NEW				0
#define RECIEVING		1
#define	R_CHUNKS		2
#define	RECIEVED		3
#define	SENDING			4
#define	S_CHUNKS		5

class Client
{
	public:
		Client();
		//~Client();
		bool parse_first_line();
		bool parseHeaders(std::string &Buffer);
		bool headersValidate(std::string &Buffer, std::string method);
		bool validatePostHeaders(std::string &buffer);
		bool validateGetHeaders(std::string &buffer);
		int validateContentLength();
		int validateHost();
		int validateTransferEncoding();
		bool bodyValidate(std::string &Buffer);
		bool handleChunkedTransferEncoding(std::istringstream &headerStream);
		bool handleContentLength(std::istringstream &headerStream);
		bool handlingBody(std::string &body);
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
		bool getSentHeader() { return _sentHeader; }
		void setSentHeader(bool value) { _sentHeader = value; }
		void setResponse(Response &response) { _response = response; }
		Response &getResponse() { return _response; }
		void setHostPort(std::string hostPort) { _hostPort = hostPort; }
		std::string getHostPort(std::string hostPort) { return _hostPort; }

		int status;
		bool getIsChunked() { return _isChunked; }
		void setClientTime(std::time_t initTime) {_timeOut = initTime;}
		std::time_t getClientTime(){return _timeOut;}
		void saveBodyToFile();

	
	private:
	std::time_t _timeOut;
	std::string _buffer;			// for storing and processing request header & body; more accurate name: _requestBuffer
	std::string _responseBuffer;	// for storing and processing response header & body
	int _clientFD;
	parser request;
	std::string _boundary;
	bool _isChunked;
	bool _headersIsChunked;
	bool _newLineChecked;
	bool _chunkLengthRecieved;
	int  _chunkLengthValue;
	size_t _targetBytes;
	size_t _bytesRead;
	bool isAllRecieved;
	bool _sentHeader; // for response in chunks
	Response _response;
	std::string _hostPort;
};


size_t stringToSizeT(const std::string& str);
bool isHexadecimal(const std::string& str);
int hexStringToInt(const std::string& hexStr);
