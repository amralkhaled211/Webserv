#pragma once
#include "Client.hpp"
#include "StatusMsg.hpp"
#include "Response.hpp"
// #include "CGI.hpp"

struct parser;
class Client;

#define	SD_OK				0
#define	SD_NO_READ_PERM		-1
#define	SD_NO_FILE			-2

#define	PATH_TO_WWW	"/home/aismaili/Webserv/"
// "/home/aismaili/Webserv/"

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


class SendData
{
	private:
	Response _response;
	Redirection _redir;
	bool _isDir;
	bool _isReturn;
	StatusMsg _status;
	
	public:
	class LocationNotFoundException : public std::exception
	{
	};
	void notfound();
	bool read_file(std::string const &path);
	LocationBlock findLocationBlock(std::vector<LocationBlock> &locations, parser &request);
	template <typename T>
	bool findInVector(const std::vector<T> &vec, const T &target)
	{
		return std::find(vec.begin(), vec.end(), target) != vec.end();
	}
	ServerBlock findServerBlock(std::vector<ServerBlock> &servers, parser &request);
	bool findIndexFile(const std::vector<std::string> &files, std::string &root);
	std::string findCGIIndex(const std::vector<std::string> &files, std::string &root);
	void handleCGI(const std::string &root, parser &request, ServerBlock server, LocationBlock location);
	int	checkCGIFile(const std::string &path);
	bool isCGI(LocationBlock location);
	bool checkDeletePath(std::string path, LocationBlock location);
	void redirect(LocationBlock& location, parser &request);
	bool saveBodyToFile(const std::string &filename, parser &request);
	Response &sendResponse(std::vector<ServerBlock> &servers, parser &request);
	void displayDir(const std::string& path, const std::string& requestPath);
	void createResponseHeader(int code, size_t bodySize, std::string contentTypes);
	void createDfltResponseBody(int code, std::string&	contentType, std::string postFix = "html");
	void codeErrorResponse(int code);
	std::string getErrorPagePath(int code, const std::vector<std::vector<std::string> >& errorPage);
	int readFromErrorPage(std::string& errorPagePath, std::string& body);
	bool isNotAllowedMethod(LocationBlock& location, std::vector<std::string> allowedMethods, std::string method);


	template<typename T>
	void		prepErrorResponse(int code, T& location)
	{
		std::string		errorPagePath = PATH_TO_WWW + getErrorPagePath(code, location.getErrorPage());
		std::string		contentType;
		int				fileStatus;

		//std::cout << "IN PREP ERROR RESPONSE\n";

		if (!errorPagePath.empty())
		{
			removeExcessSlashes(errorPagePath);

			std::cout << "ERROR PAGE PATH: " << errorPagePath << std::endl;

			fileStatus = readFromErrorPage(errorPagePath, _response.body); // this already writes into the body (passed by reference)
			if (fileStatus == SD_OK) { // body gets init with right error_page content
				contentType = mimeTypesMap_G[get_file_extension(errorPagePath)];

				if (contentType == "")
					createDfltResponseBody(code, contentType, "txt");
			}
			else if (fileStatus == SD_NO_READ_PERM) {
				if (code != 403) {
					prepErrorResponse(403, location);
					return ;
				}
				else
					createDfltResponseBody(code, contentType);
			}
			else if (fileStatus == SD_NO_FILE) {
				if (code != 404) {
					prepErrorResponse(404, location);
					return ;
				}
				else
					createDfltResponseBody(code, contentType);
			}
		}
		else
			createDfltResponseBody(code, contentType);

		createResponseHeader(code, _response.body.size(), contentType);
	}
};


// #include "ErrorTemplate.tpp"