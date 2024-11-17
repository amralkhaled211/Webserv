#pragma once
#include "RequestHandler.hpp"
#include "StatusMsg.hpp"

#define	SD_OK	0
#define	SD_NO_READ_PERM	-1
#define	SD_NO_FILE			-2

typedef struct Redirection
{
    std::map<std::string, std::string> CodeToMessage;

    Redirection()
	{
        CodeToMessage["301"] = "Moved Permanently";
        CodeToMessage["302"] = "Found";
        CodeToMessage["307"] = "Temporary Redirect";
        CodeToMessage["308"] = "Permanent Redirect";
    }
} Redirection;

typedef struct Response
{
	std::string status;
	std::string contentType;
	std::string location;
	std::string contentLength;
	std::string body;
} Response;

class SendData
{
	private:
	Response _response;
	Redirection _redir;
	bool _isDir;
	bool _isReturn;
	StatusMsg _status;
	
	public:
	void notfound();
	bool read_file(std::string const &path, parser &request);
	LocationBlock findLocationBlock(std::vector<LocationBlock> &locations, parser &request);
	template <typename T>
	bool findInVector(const std::vector<T> &vec, const T &target)
	{
		// std::cout << BOLD_YELLOW << "FIND IN VECTOR -> size: " << vec.size() << " begin: " << *vec.begin() << RESET << std::endl;
		// std::cout << "FIND IN VECTOR -> target: " << target << std::endl;
		return std::find(vec.begin(), vec.end(), target) != vec.end();
	}
	ServerBlock findServerBlock(std::vector<ServerBlock> &servers, parser &request);
	bool findIndexFile(const std::vector<std::string> &files, std::string &root, parser &request);
	void handleCGI(const std::string &root, parser &request, ServerBlock server, LocationBlock location);
	bool isCGI(const parser &request, LocationBlock location);
	void redirect(LocationBlock& location);
	void saveBodyToFile(const std::string &filename, parser &request);
	std::string sendResponse(int clientSocket, std::vector<ServerBlock> &servers, parser &request, int epollFD);
	void displayDir(const std::string& path, const std::string& requestPath);
	void prepErrorResponse(int code, LocationBlock& locationBlock);
	void createResponseHeader(int code, int bodySize, std::string contentTypes);
	void createDfltResponseBody(int code, std::string&	contentType, std::string postFix = "html");
};