#include "SendData.hpp"
#include "CGI.hpp"

void SendData::notfound()
{
	_response.status = "HTTP/1.1 404 Not Found\r\n";
	_response.contentType = "Content-Type: text/html;\r\n";
	_response.contentLength = "Content-Length: 155\r\n";
	_response.body += "<!DOCTYPE html><html><head><title>404 Not Found</title></head>";
	_response.body += "<body><h1>404 Not Found</h1><p>The page you are looking for does not exist.</p></body></html>";
}

bool SendData::read_file(std::string const &path, parser &request) // this already prepares the response
{
	std::string file_extension = get_file_extension(path);

	std::ifstream file(path.c_str());
	if (file.is_open())
	{
		_response.body.clear();
		std::string line;
		while (std::getline(file, line))
		{
			_response.body += line + "\n";
		}
		std::string contentType = mimeTypesMap_G[file_extension];
		if (contentType == "")
			contentType = "text/plain";

		createResponseHeader(200, _response.body.size(), contentType);

		file.close();
		return true;
	}
	/* if (!file.is_open())
		std::cout << "COULDN'T OPEN FILE: " << path << std::endl; */
	return false;
}

int SendData::checkCGIFile(const std::string &path)
{
	struct stat buffer;

	if (stat(path.c_str(), &buffer) == 0)
	{
		if (access(path.c_str(), R_OK) != 0 || access(path.c_str(), X_OK) != 0)
			return 403;
		int fd = open(path.c_str(), O_RDONLY);
		if (fd == -1)
			return 404;
		close(fd);
	}
	else
		return 404;
	return 0;
}

void SendData::handleCGI(const std::string &root, parser &request, ServerBlock server, LocationBlock location) //will need to return errors from here
{
	std::string file_extension = '.' +  get_file_extension(root);
	std::vector<std::string> allowed_ext = location.getCgiExt();
	bool isAllowed = false;
	for (std::vector<std::string>::iterator it = allowed_ext.begin(); it != allowed_ext.end(); it++)
	{
		if (*it == file_extension)
		{
			isAllowed = true;
				break;
		}
	}
	if (!isAllowed)
	{
		prepErrorResponse(400, location);
		return ;
	}
	int code = checkCGIFile(root);
	if (code != 0)
	{
		//std::cout << RED << "CGI FILE ERROR: " << code << RESET <<std::endl;
		prepErrorResponse(code, location);
		return ;
	}
	CGI cgi(root, request);
	cgi.setEnv(server);
	if (cgi.setInterpreters(location) == false)
	{
		prepErrorResponse(500, location);
		return ;
	}

	code = cgi.executeScript();
	//std::cout << RED << "Done with CGI execution" << RESET << std::endl;
	if (code != 0)
	{
		prepErrorResponse(code, location);
		return ;
	}

	cgi.generateResponse();
	//std::cout << RED << "Done with CGI response" << RESET << std::endl;

	_response.body = cgi.getResponse();

	if (cgi.getStatusSet())
	{
		//std::cout << MAGENTA_COLOR << "Status set" << RESET << std::endl;
		_response.status = cgi.getResponseStatus() + "\r\n";
	}
	else
		_response.status = "HTTP/1.1 200 OK\r\n";

	if (cgi.getTypeSet())
	{
		//std::cout << MAGENTA_COLOR << "Content type set" << RESET << std::endl;
		_response.contentType = cgi.getContentType() + ";" + "\r\n";
	}
	else
		_response.contentType = "Content-Type: text/html;\r\n";
		
	if (_response.body.size() > SEND_CHUNK_SIZE) {
		_response.transferEncoding = "Transfer-Encoding: chunked\r\n";
		_response.contentLength = "";
	}
	else {
		_response.contentLength = "Content-Length: " + intToString(_response.body.size()) + "\r\n";
		_response.transferEncoding = "";
	}
}

// std::vector<std::string>	possibleRequestedLoc(std::string uri) {
// 	std::vector<std::string>	possibleReqLoc;
// 	size_t						lastSlash;

// 	removeExcessSlashes(uri);

// 	do
// 	{
// 		possibleReqLoc.push_back(uri);
// 		lastSlash = uri.find_last_of('/');
// 		uri = uri.substr(0, lastSlash);
// 	} while (!uri.empty());

// 	if (possibleReqLoc[possibleReqLoc.size() - 1] != "/")
// 		possibleReqLoc.push_back("/");

// 	return possibleReqLoc;
// }


LocationBlock SendData::findLocationBlock(std::vector<LocationBlock> &locations, parser &request)
{
	std::vector<std::string> possibleReqLoc = possibleRequestedLoc(request.path); // other name: possibleReqLoc
	LocationBlock	location;
	std::string		fullPath;

	_isDir = true;
	
	for (int i = 0; i < possibleReqLoc.size(); ++i)
	{
		for (std::vector<LocationBlock>::iterator it = locations.begin(); it != locations.end(); ++it)
		{
			location = *it;

			if (location.getPrefix() == possibleReqLoc[i]) // need to make sure the prefix is also cleaned from excess slashes
			{
				fullPath = PATH_TO_WWW + location.getRoot() + '/' + possibleReqLoc[0]; // for defining whether request is a directory or a file
				if (isDirectory(fullPath))
					_isDir = true;
				else
					_isDir = false;

				return location;
			}
		}
	}
	// serve default page in this case
	throw LocationNotFoundException();
	std::cout << BOLD_RED << "COULD NOT FIND LOCATION BLOCK" << RESET << std::endl;
	// throw std::exception(); // this is temporary, will create a error handling mechanism
}

ServerBlock SendData::findServerBlock(std::vector<ServerBlock> &servers, parser &request) // uses the Host header field -> server_name:port -> Host: localhost:8081
{
	std::string host = request.headers["Host"];
	size_t colon_pos = host.find(':');
	std::string server_name = (colon_pos != std::string::npos) ? host.substr(0, colon_pos) : host;
	std::string port = (colon_pos != std::string::npos) ? host.substr(colon_pos + 1) : "";

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		ServerBlock &server = *it;
		if (findInVector(server.getListen(), stringToInt(port)) && findInVector(server.getServerName(), server_name)) // here port is prioritized over server_name
			return server;
	}
	// this would be fixed later
	// std::cout << "reques:::" << request.headers["Host"] << std::endl;
	//std::cout << "\033[1;31m" <<  "returning the first server?, This is a BUG " << "\033[0m" << std::endl;
	return servers[0]; // return default
}

void SendData::redirect(LocationBlock& location, parser &request) // so far handling url redirection, relative will be handled soon
{
	std::vector<std::string> returnVec = location.getReturn();
	std::string code;
	std::string url;
	bool isExternal = false;
	code = returnVec[0];
	url = returnVec[1];
	if (url.find("http://") == 0 || url.find("https://") == 0)
	    isExternal = true;
	if (isExternal)
	{
	    // Handle external URL redirection
	    _response.status = "HTTP/1.1 " + code + " " + _redir.CodeToMessage[code] + "\r\n";
	    _response.location = "Location: " + url + "\r\n";
	    _response.contentType = "Content-Type: text/html;\r\n";
	    _response.contentLength = "Content-Length: 0\r\n";
	    _response.body = "";
	}
	else
	{
		//std::cout << "url: " << url << std::endl;
	    // Handle internal redirection
	    _response.status = "HTTP/1.1 " + code + " " + _redir.CodeToMessage[code] + "\r\n";
	    _response.location = "Location: http://" + request.headers["Host"] + url + "\r\n";
	    _response.contentType = "Content-Type: text/html;\r\n";
	    _response.contentLength = "Content-Length: 0\r\n";
	    _response.body = ""; // You may want to set the body to the content of the internal page
	}
}

bool SendData::findIndexFile(const std::vector<std::string> &files, std::string &root, parser &request) // if found will also prepare response, else return false
{
	size_t i = 0;

	while (i < files.size())
	{
		std::string file = root + '/' + files[i];
		removeExcessSlashes(file);
		//std::cout << BOLD_GREEN << "FILE from index: " << file << RESET << std::endl;
		//std::cout << BOLD_GREEN << "FILE from index: " << file << RESET << std::endl;
		if (read_file(file, request))
			return true;
		i++;
	}
	return false;
}

std::string SendData::findCGIIndex(const std::vector<std::string> &files, std::string &root, parser &request) // if found will also prepare response, else return false
{
	bool found = false;
	size_t i = 0;
	std::string file;

	while (i < files.size())
	{
		file = root + '/' + files[i];
		removeExcessSlashes(file);
		struct stat		buffer;
		//std::cout << "FILE: " << file << std::endl;

		if (stat(file.c_str(), &buffer) == 0 && access(file.c_str(), R_OK) == 0 && access(file.c_str(), X_OK) == 0)
		{
			found = true;
			break;
		}
		else	
			i++;
	}
	if (found)
	{	
		return file;
	}
	return "";
}

bool SendData::isCGI(LocationBlock location)
{
	if (location.getCgiPath().empty() || location.getCgiExt().empty())
		return false;
	return true;
}

bool	SendData::isNotAllowedMethod(LocationBlock& location, std::vector<std::string> allowedMethods, std::string currMethod)
{
	std::vector<std::string>::iterator itAllowedMethod = std::find(allowedMethods.begin(), allowedMethods.end(), currMethod);
	if (itAllowedMethod == allowedMethods.end())
	{
		prepErrorResponse(405, location);
		return true;
	}
	return false;
}

bool SendData::checkDeletePath(std::string path, LocationBlock location)
{
    size_t lastSlashPos = path.find_last_of('/');
    if (lastSlashPos == std::string::npos) {
        return false;
    }

    std::string directory = path.substr(0, lastSlashPos);

    if (directory.size() >= 7 && directory.substr(directory.size() - 7) == "/upload") {
        std::string root = PATH_TO_WWW + location.getRoot() + directory;
        if (access(root.c_str(), W_OK) == 0) {
            return true;
        }
    }
    return false;
}

Response &SendData::sendResponse(int clientSocket, std::vector<ServerBlock> &servers, parser &request, int epollFD)
{

	//here  i would serve the errpr pages if the request is not valid
	if (request.statusError != 0)
	{
		codeErrorResponse(request.statusError);
		return _response;
	}

	request.path = decodeURIComponent(request.path);
	request.queryString = decodeURIComponent(request.queryString, true);
	
	_isReturn = false;
	ServerBlock current_server = findServerBlock(servers, request);

	LocationBlock location;
	try
	{
		location = findLocationBlock(current_server.getLocationVec(), request);
	}
	catch(const LocationNotFoundException& e)
	{
		std::string contentType;
		createDfltResponseBody(200, contentType);
		createResponseHeader(200, _response.body.size(), contentType);
		return _response;
	}

	if (request.method == "GET")
	{
		try
		{			
			if (isNotAllowedMethod(location, location.getAllowedMethods(), "GET"))
				return _response;

			std::string root = PATH_TO_WWW + location.getRoot() + request.path; // maybe a more suitable name: pathToFileToServe
			
			// std::cout << MAGENTA_COLOR << "Root: " << root << std::endl << "Request path:" <<  request.path << RESET << std::endl;
			/* location.printLocationBlock(); */

			if (location.getReturn().empty())
			{
				if (_isDir == true) // here we handle the directory
				{
					if (isCGI(location)) //check if cgi location then if found the index file execute the cgi else execute the directory 
					{
						std::string file = findCGIIndex(location.getIndex(), root, request);
						if (file != "")
							handleCGI(file, request, current_server, location);
						else if (file == "" && location.getAutoindex() == ON) {
							this->displayDir(root, request.path);
						}
						else if (file == "")
							prepErrorResponse(404, location);
					}
					else //checking again if there is an index file, else display the directory
					{
						bool foundFile = findIndexFile(location.getIndex(), root, request);
						if (!foundFile && location.getAutoindex() == ON) {
							this->displayDir(root, request.path);
						}
						else if (!foundFile)
							prepErrorResponse(404, location);
					}
				}
				else if (isCGI(location))
				{
					handleCGI(root, request, current_server, location);
				}
				else
				{
					if (!this->read_file(root, request)) {
						//DEBUG_R "sending 404 error!!" << RESET << std::endl;
						prepErrorResponse<LocationBlock>(404, location);
					}
				}
			}
			else
			{
				redirect(location, request);
				_isReturn = true;
			}
		}
		catch (const std::exception &e) // what kind of error do we expect here?
		{
			_response.body.clear();
			_response.status.clear();
			_response.contentType.clear();
			_response.contentLength.clear();
			//std::cout << YELLOW << "Sending 404" << RESET << std::endl;
			prepErrorResponse<ServerBlock>(404, current_server);
		}
	}
	if (request.method == "POST")
	{
		LocationBlock location = findLocationBlock(current_server.getLocationVec(), request);
		
		if (isNotAllowedMethod(location, location.getAllowedMethods(), "POST"))
				return _response;

		std::string root = PATH_TO_WWW + location.getRoot() + request.path;
		//std::cout << MAGENTA_COLOR << "Root: " << root << std::endl << "Request path:" <<  request.path << std::endl << "Request method: " << request.method << RESET << std::endl;

		//std::cout << CYAN_COLOR << "Request body: " << request.body << RESET << std::endl;


		if (isCGI(location)) // might need to rethink this, eg. if resource for video.py is in cgi-bin it wont output the video beacuse it thinks its not an acceptable extension
		{
			//std::cout << RED_COLOR << "In CGI POST" << RESET << std::endl;
			//std::cout << root << std::endl;
			handleCGI(root, request, current_server, location);
		}
		else
		{
			// saveBodyToFile("../website/upload/" + request.fileName, request);
			_response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
			_response.body += "<body><h1>200 OK</h1><p>file saved</p></body></html>";
			this->createResponseHeader(200, _response.body.size(), mimeTypesMap_G["html"]);
		}
	}
	else if (request.method == "DELETE")
	{
		if (isNotAllowedMethod(location, location.getAllowedMethods(), "DELETE"))
				return _response;
		std::string root = PATH_TO_WWW + location.getRoot() + request.path; // maybe a more suitable name: pathToFileToServe

		//std::cout << BOLD_RED << "IN DELETE METHOD" << RESET << std::endl;

		//std::cout << MAGENTA_COLOR << "Path: " << request.path << std::endl;

		if (checkDeletePath(request.path, location))
		{
			struct stat buffer;
			if (stat(root.c_str(), &buffer) == 0) 
			{
				if (remove(root.c_str()) == 0) 
				{
					_response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
					_response.body += "<body><h1>200 OK</h1><p>File deleted successfully</p></body></html>";
					_response.status = "HTTP/1.1 200 OK\r\n";
					_response.contentType = "Content-Type: text/html;\r\n";
					_response.contentLength = "Content-Length: " + intToString(_response.body.size()) + "\r\n";
				}
				else
				{
					std::cout << BOLD_RED << "Error deleting file" << RESET << std::endl;
					prepErrorResponse(500, location);
				}
			}
			else
				prepErrorResponse(404, location);
		}
		else
		{
			std::cout << std::endl << BOLD_RED << "You have no right to delete" << RESET << std::endl;
			prepErrorResponse(403, location);
		}
	}
	// std::cout << MAGENTA_COLOR <<  _response.status << std::endl;
	// std::cout << _response.contentType << std::endl;
	// std::cout << _response.contentLength << RESET << std::endl;
	return _response;
}


bool SendData::saveBodyToFile(const std::string &filename, parser &request)
{
    std::ofstream outFile(filename.c_str(), std::ios::binary);//| std::ios::app
    if (outFile.is_open())
    {
        outFile.write(request.body.c_str(), request.body.size());
        outFile.close();
		request.body.clear();
		return true;
    }
    else
    {
        // Handle error opening file
        std::cerr << "Error opening file for writing: " << filename << std::endl;
		request.body.clear();
		return false;
    }
}

// Percent-encode a filename for use in a URI
std::string encodeURI(const std::string& filename) {
    std::ostringstream encoded;
    for (size_t i = 0; i < filename.length(); ++i) {
		char c = filename[i];
        if (isalnum(c) || c == '-' || c == '_' || 
            c == '.' || c == '~' || c == '/') {
            encoded << c;
        }
		else if (c == ':' || c == '@' || c == '!' || c == '$' || c == '&' || c == '\'' ||
                   c == '(' || c == ')' || c == '*' || c == '+' || c == ',' || c == ';' || c == '=') {
            encoded << c; // Do not encode reserved characters
		}
		else {
            encoded << '%' << std::hex << std::uppercase << std::setw(2) 
                    << std::setfill('0') << (static_cast<int>(c) & 0xFF);
        }
    }
    return encoded.str();
}

std::vector<std::pair<std::string, std::string> >	listDirectory(const std::string& path)
{
	std::vector<std::pair<std::string, std::string> >	elements;
	DIR*	dir = opendir(path.c_str());
	if (dir == NULL) {
		// it can return NULL, if no permission to open directory!
		// test this scenario with nginx, expected: 403 Permission Denied
		return elements;
	}

	struct dirent* entry;
	std::string	name;
	std::string	fullPath;
	while ((entry = readdir(dir)) != NULL) {
		name = entry->d_name;
		if (name != "." && (name == ".." || name[0] != '.')) { // not accepting hidden files, except of ".."
			fullPath = path + '/' + name;
			removeExcessSlashes(fullPath);
			struct stat statbuf;
			if (stat(fullPath.c_str(), &statbuf) == 0) {
				if (S_ISDIR(statbuf.st_mode))
					fullPath += '/';
				elements.push_back(std::make_pair(name, fullPath));
			}
		}
	}
	closedir(dir);
	return elements;
}

std::string escapeHtml(const std::string &input)
{
	std::string output;
	
	for (size_t i = 0; i < input.size(); ++i) {
		switch (input[i]) {
			case '&':  output += "&amp;";
				break;
			case '<':  output += "&lt;";
				break;
			case '>':  output += "&gt;";
				break;
			case '"':  output += "&quot;";
				break;
			case '\'': output += "&#39;";
				break;
			default:   output += input[i];
				break;
		}
	}
	return output;
}

void		SendData::displayDir(const std::string& path, const std::string& requestURI)
{
	// first pair-element is the element, second one is the full path, but with a '/' at the end for directories
	std::vector<std::pair<std::string, std::string> >	dirElements(listDirectory(path));

	// must embed the dirElements into a html file
	std::ostringstream html;
	html << "<!DOCTYPE html><html><head><title>Index of " << escapeHtml(requestURI) << "</title></head><body>";
	html << "<h1>Index of " << escapeHtml(requestURI) << "</h1>";
	html << "<ul>";

	html << "<table border=\"1\">";
	html << "<tr><th>File Name</th><th>Size (bytes)</th><th>Last Modified</th></tr>";

	for (size_t i = 0; i < dirElements.size(); ++i) {
		std::string displayName = dirElements[i].first;
		std::string fullPath = requestURI;
		if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
			fullPath += '/';
		fullPath += displayName;

		struct stat fileStat;
		if (stat(dirElements[i].second.c_str(), &fileStat) == 0) {
			// Get file size
			off_t fileSize = fileStat.st_size;

			// Get last modification time
			std::time_t modTime = fileStat.st_mtime;
			std::tm *tm = std::localtime(&modTime);
			char timeBuffer[20];
			std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", tm);
			std::string modTimeStr(timeBuffer);

			// Include size and last modification date in the HTML output
			std::stringstream htmlStream;
			htmlStream << "<tr><td><a href=\"" << escapeHtml(fullPath) << "\">" << escapeHtml(displayName) << "</a></td>"
					<< "<td>" << fileSize << "</td>"
					<< "<td>" << modTimeStr << "</td></tr>";
			html << htmlStream.str();
		} else {
			std::cout << "ERRNO: " << errno << std::endl;
			std::stringstream htmlStream;
			htmlStream << "<tr><td><a href=\"" << escapeHtml(fullPath) << "\">" << escapeHtml(displayName) << "</a></td>"
					<< "<td colspan=\"2\">(unable to retrieve file information)</td></tr>";
			html << htmlStream.str();
		}
	}

	html << "</table>";

	html << "</ul></body></html>";

	// embed created body inside response struct
	_response.body = html.str();
	// std::cerr << _response.body << std::endl;
	size_t content_len = _response.body.size();
	createResponseHeader(200, content_len, "text/html");
}


bool			codeDefinedInErrorPage(int code, const std::vector<std::string>& errorPage)
{
	std::stringstream	ss;
	int					errCode;

	for (size_t i = 0; i < errorPage.size() - 1; ++i) {
		ss << errorPage[i];
		ss >> errCode;
		if (errCode == code)
			return true;
	}
	return false;
}

std::string		SendData::getErrorPagePath(int code, const std::vector<std::vector<std::string> >& errorPage)
{
	if (errorPage.empty())
		return "";
	for (size_t i = 0; i < errorPage.size(); ++i) {
		if (codeDefinedInErrorPage(code, errorPage[i]))
			return errorPage[i].back();
	}
	return "";
}

void		SendData::createResponseHeader(int code, size_t bodySize, std::string contentTypes)
{
	_response.status = "HTTP/1.1 " + intToString(code) + " " + _status._statusMsg[code][0] + "\r\n";
	_response.contentType = "Content-Type: " + contentTypes + ";\r\n";
	if (bodySize > SEND_CHUNK_SIZE) {
		_response.transferEncoding = "Transfer-Encoding: chunked\r\n";
		_response.contentLength = "";
	}
	else {
		_response.contentLength = "Content-Length: " + intToString(bodySize) + "\r\n";
		_response.transferEncoding = "";
	}
}


int		SendData::readFromErrorPage(std::string& errorPagePath, std::string& body)
{
	struct stat		buffer;

	if (stat(errorPagePath.c_str(), &buffer) != 0)
		return SD_NO_FILE;

	if (access(errorPagePath.c_str(), R_OK) != 0)
		return SD_NO_READ_PERM;

	std::ifstream	file(errorPagePath.c_str());

	if (!file.is_open()) // should never go here, because of above checks! maybe will remove later
		return SD_NO_FILE;

	std::stringstream	ss;
	ss << file.rdbuf();

	body = ss.str();
	//std::cout << "BODY: " << body << std::endl;
	return SD_OK;
}

void	SendData::codeErrorResponse(int code)
{
	std::string contentType;
	createDfltResponseBody(code, contentType);
	createResponseHeader(code, _response.body.size(), contentType);
}

void	SendData::createDfltResponseBody(int code, std::string&	contentType, std::string postFix) {
	_response.body = "<!DOCTYPE html><html><head><title>" + intToString(code) + " " + _status._statusMsg[code][0]/*  + " Not Found" */ + "</title></head>";
	_response.body += "<body><h1>" + intToString(code) + " " + _status._statusMsg[code][0] + "</h1><p>" + _status._statusMsg[code][1] + "</p></body></html>";
	contentType = mimeTypesMap_G[postFix];
}
