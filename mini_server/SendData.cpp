#include "SendData.hpp"
#include "CGI.hpp"
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
	/* std::cout << "-----------------------------------\n"
	<<"file extention: " << file_extension << std::endl
	<< "request path: " << request.path << "\n"
	<< "path: " << path
	<< "-----------------------------------\n" ; */
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

		// _response.status = "HTTP/1.1 200 OK\r\n";
		// _response.contentType = "Content-Type: " + mimeTypesMap_G[file_extension] + ";" + "\r\n";
		// unsigned int content_len = _response.body.size();
		// _response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";

		file.close();
		return true;
	}
	/* if (!file.is_open())
		std::cout << "COULDN'T OPEN FILE: " << path << std::endl; */
	return false;
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
	CGI cgi(root, request);
	cgi.setEnv(server);
	if (cgi.setInterpreters(location) == false)
	{
		prepErrorResponse(500, location);
		return ;
	}

	int code = cgi.executeScript();
	if (code != 0)
	{
		prepErrorResponse(code, location);
		return ;
	}

	cgi.generateResponse();

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
		
	unsigned int content_len = _response.body.size();
		_response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
}

std::vector<std::string>	possibleRequestedLoc(std::string uri) {
	std::vector<std::string>	possibleReqLoc;
	size_t						lastSlash;

	removeExcessSlashes(uri);

	do
	{
		possibleReqLoc.push_back(uri);
		lastSlash = uri.find_last_of('/');
		uri = uri.substr(0, lastSlash);
	} while (!uri.empty());

	if (possibleReqLoc[possibleReqLoc.size() - 1] != "/")
		possibleReqLoc.push_back("/");

	return possibleReqLoc;
}

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
				fullPath = location.getRoot() + '/' + possibleReqLoc[0]; // for defining whether request is a directory or a file
				if (isDirectory(fullPath))
					_isDir = true;
				else
					_isDir = false;

				return location;
			}
		}
	}
	std::cout << BOLD_RED << "COULD NOT FIND LOCATION BLOCK" << RESET << std::endl;
	throw std::exception(); // this is temporary, will create a error handling mechanism
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
	/* std::cout << "       HOST: " << host << std::endl;
	std::cout << "SERVER_NAME: " << server_name << std::endl;
	std::cout << "       PORT: " << port << std::endl;
	print_map(request.headers); */
	//std::cout << "\033[1;31m" <<  "returning the first server?, This is a BUG " << "\033[0m" << std::endl;
	return servers[0]; // return default
}

void SendData::redirect(LocationBlock& location) // so far handling url redirection, relative will be handled soon
{
	std::vector<std::string> returnVec = location.getReturn();
	std::string code;
	std::string url;
	if (returnVec.size() > 1)
	{
		code = returnVec[0];
		url = returnVec[1];
	}
	else
	{
		code = "307";
		url = returnVec[0];
	}
	_response.status = "HTTP/1.1 " + code + " " + _redir.CodeToMessage[code] + "\r\n";
	_response.location = "Location: " + url + "\r\n";
	_response.contentType = "Content-Type: text/html;\r\n";
	_response.contentLength = "Content-Length: 0\r\n";
	_response.body = "";
}

bool SendData::findIndexFile(const std::vector<std::string> &files, std::string &root, parser &request) // if found will also prepare response, else return false
{
	size_t i = 0;

	while (i < files.size())
	{
		std::string file = root + '/' + files[i];
		removeExcessSlashes(file);
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
		std::cout << "FILE: " << file << std::endl;

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

bool SendData::isCGI(const parser &request, LocationBlock location)
{
	if (location.getCgiPath().empty() || location.getCgiExt().empty())
		return false;
	return true;
}

Response &SendData::sendResponse(int clientSocket, std::vector<ServerBlock> &servers, parser &request, int epollFD)
{
	_isReturn = false;

	ServerBlock current_server = findServerBlock(servers, request);

	if (request.method == "GET")
	{
		try
		{
			LocationBlock location = findLocationBlock(current_server.getLocationVec(), request);

			std::string root = location.getRoot() + request.path; // maybe a more suitable name: pathToFileToServe
			
			//std::cout << MAGENTA_COLOR << "Root: " << root << std::endl << "Request path:" <<  request.path << RESET << std::endl;
			/* location.printLocationBlock(); */
			
			if (location.getReturn().empty())
			{
				if (_isDir == true) // here we handle the directory
				{
					if (isCGI(request, location)) //check if cgi location then if found the index file execute the cgi else execute the directory 
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
				else if (isCGI(request, location)) // add a check here that also checks if the extensions exist and match to avoid going into cgi if unnecessary
				{
					//std::cout << RED_COLOR << "In CGI GET" << RESET << std::endl;
					handleCGI(root, request, current_server, location);
				}
				else
				{
					if (!this->read_file(root, request)) {
						DEBUG_R "sending 404 error!!" << RESET << std::endl;
						prepErrorResponse(404, location);
					}
				}
			}
			else
			{
				redirect(location);
				_isReturn = true;
			}
		}
		catch (const std::exception &e) // what kind of error do we expect here?
		{
			std::string error = e.what(); // here we need to check what the error is and send notfound or error page accordingly
			_response.body.clear();
			_response.status.clear();
			_response.contentType.clear();
			_response.contentLength.clear();
			std::cout << YELLOW << "Sending 404" << RESET << std::endl;
			notfound();
		}
	}
	if (request.method == "POST")
	{
		LocationBlock location = findLocationBlock(current_server.getLocationVec(), request);
		std::string root = location.getRoot() + request.path;
		//std::cout << MAGENTA_COLOR << "Root: " << root << std::endl << "Request path:" <<  request.path << std::endl << "Request method: " << request.method << RESET << std::endl;

		if (isCGI(request, location)) // might need to rethink this, eg. if resource for video.py is in cgi-bin it wont output the video beacuse it thinks its not an acceptable extension
		{
			//std::cout << RED_COLOR << "In CGI POST" << RESET << std::endl;
			handleCGI(root, request, current_server, location);
		}
		else
		{
			saveBodyToFile("../website/upload/" + request.fileName, request);
			_response.status = "HTTP/1.1 200 OK\r\n";
			_response.contentType = "Content-Type: text/html;\r\n";
			_response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
			_response.body += "<body><h1>200 OK</h1><p>file saved</p></body></html>";
			_response.contentLength = "Content-Length: " + intToString(_response.body.size()) + "\r\n";
		}
	}

	// std::string resp;

	// if (_isReturn)
	// 	resp = _response.status + _response.location  + _response.contentType + _response.transferEncoding + _response.contentLength + "\r\n" + _response.body;
	// else
	// 	resp = _response.status + _response.contentType + _response.transferEncoding + _response.contentLength + "\r\n" + _response.body;

	// this makes sure we are able to send() on the next epoll() iteration
	// struct epoll_event client_event;
    // client_event.data.fd = clientSocket;
    // client_event.events = EPOLLOUT;
	// if (epoll_ctl(epollFD, EPOLL_CTL_MOD, clientSocket, &client_event) == -1)
    // {
    //     close(clientSocket);
	// 	std::cout << BOLD_GREEN << "clientSocket Change mod : " << clientSocket << RESET << std::endl;
	// 	std::cout << "epoll_ctl failed" << std::endl;
    //     throw std::runtime_error("in sendResponse(): epoll_ctl while MODIFYING client FD " + intToString(clientSocket));
    // }
	/* std::cout << BLUE_COLOR << "sending response " << RESET << std::endl;
	std::cout << resp << std::endl; */
	return _response;
}


void SendData::saveBodyToFile(const std::string &filename, parser &request)
{
    std::ofstream outFile(filename.c_str(), std::ios::binary);
    if (outFile.is_open())
    {
        outFile.write(request.body.c_str(), request.body.size());
        outFile.close();
    }
    else
    {
        // Handle error opening file
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
	request.body.clear();
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

#include <ctime>

void		SendData::displayDir(const std::string& path, const std::string& requestPath)
{
	// first pair-element is the element, second one is the full path, but with a '/' at the end for directories
	std::vector<std::pair<std::string, std::string> >	dirElements(listDirectory(path));

	// std::cout << "Elements in directory " << path << ":" << std::endl;
	// std::cout << "Dir/File Name" << std::setw(30) << "Dir/File Path\n";
	// for (size_t i = 0; i < dirElements.size(); ++i) {
	// 	std::cout << dirElements[i].first << std::setw(50) << dirElements[i].second << std::endl;
	// }

	// must embed the dirElements into a html file
	std::ostringstream html;
	html << "<!DOCTYPE html><html><head><title>Index of " << escapeHtml(requestPath) << "</title></head><body>";
	html << "<h1>Index of " << escapeHtml(requestPath) << "</h1>";
	html << "<ul>";

	html << "<table border=\"1\">";
	html << "<tr><th>File Name</th><th>Size (bytes)</th><th>Last Modified</th></tr>";

	for (size_t i = 0; i < dirElements.size(); ++i) {
		std::string displayName = dirElements[i].first;
		std::string fullPath = requestPath;
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
			// Handle error if stat() fails
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
	// _response.status = "HTTP/1.1 200 OK\r\n";
	// _response.contentType = "Content-Type: text/html;\r\n";
	// _response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
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

std::string		getErrorPagePath(int code, const std::vector<std::vector<std::string> >& errorPage)
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

int		readFromErrorPage(std::string& errorPagePath, std::string& body)
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


void		SendData::prepErrorResponse(int code, LocationBlock& location)
{
	std::string		errorPagePath = getErrorPagePath(code, location.getErrorPage());
	// std::cout << BOLD_RED << "errorPagePath: " << errorPagePath << RESET << "\n";
	std::string		contentType;
	int				fileStatus;

	//std::cout << "IN PREP ERROR RESPONSE\n";

	if (!errorPagePath.empty())
	{ // need to create error response from errorPage
		// errorPagePath = location.getRoot() + location.getPrefix() + "/" + errorPagePath;
		removeExcessSlashes(errorPagePath);
		// std::cout << BOLD_RED << "errorPagePath: " << errorPagePath << RESET << "\n";

		//std::cout << "ERROR PAGE PATH: " << errorPagePath << std::endl;

		fileStatus = readFromErrorPage(errorPagePath, _response.body); // this already reads into the body (passed by reference)
		if (fileStatus == SD_OK) { // body gets init with right error_page content
			contentType = mimeTypesMap_G[get_file_extension(errorPagePath)];

			//std::cout << "CONTENT TYPE: -->" << contentType << "<--" << std::endl;

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
	else // need to create hardcoded error response
		createDfltResponseBody(code, contentType);

	createResponseHeader(code, _response.body.size(), contentType);
}

void		SendData::createDfltResponseBody(int code, std::string&	contentType, std::string postFix) {
	_response.body = "<!DOCTYPE html><html><head><title>" + intToString(code) + " " + _status._statusMsg[code][0]/*  + " Not Found" */ + "</title></head>";
	_response.body += "<body><h1>" + intToString(code) + " " + _status._statusMsg[code][0] + "</h1><p>" + _status._statusMsg[code][1] + "</p></body></html>";
	contentType = mimeTypesMap_G[postFix];
}
