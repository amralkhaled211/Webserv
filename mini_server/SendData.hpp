#pragma once
#include "RequestHandler.hpp"

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
	
	public:
	void notfound();
	bool read_file(std::string const &path, parser &request);
	LocationBlock findLocationBlock(std::vector<LocationBlock> &locations, parser &request);
	template <typename T>
	bool findInVector(const std::vector<T> &vec, const T &target)
	{
		return std::find(vec.begin(), vec.end(), target) != vec.end();
	}
	ServerBlock findServerBlock(std::vector<ServerBlock> &servers, parser &request);
	bool findIndexFile(const std::vector<std::string> &files, std::string &root, parser &request);
	void redirect(LocationBlock& location);
	void saveBodyToFile(const std::string &filename, parser &request);
	void sendResponse(int clientSocket, std::vector<ServerBlock> &servers, parser &request);
};