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
	std::string file_extension = get_file_extension(request.path);
	std::ifstream file(path.c_str());
	if (file.is_open())
	{
		_response.body.clear();
		std::string line;
		while (std::getline(file, line))
		{
			_response.body += line + "\n";
		}
		_response.status = "HTTP/1.1 200 OK\r\n";
		_response.contentType = "Content-Type: " + mimeTypesMap_G[file_extension] + ";" + "\r\n";
		unsigned int content_len = _response.body.size();
		_response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
		file.close();
		return true;
	}
	return false;
}

std::vector<std::string>	possibleRequestedLoc(std::string uri) {
	std::vector<std::string>	possibleReqLoc;
	size_t						lastSlash;
	// remove excess slashes -> this should be done at the request parsing, to avoid repetitive work

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
	
	// for (size_t i = 0; i < possibleReqLoc.size(); ++i)
	// 	std::cout << "possibleReqLoc: " << possibleReqLoc[i] << std::endl;

	_isDir = true;

	LocationBlock	location;
	std::string		fullPath;
	
	for (int i = 0; i < possibleReqLoc.size(); ++i)
	{
		for (std::vector<LocationBlock>::iterator it = locations.begin(); it != locations.end(); ++it)
		{
			location = *it;

			if (location.getPrefix() == possibleReqLoc[i]) // need to make sure the prefix is also cleaned from excess slashes
			{
				std::cout << "location prefix: " << location.getPrefix() << std::endl;
				std::cout << "possibleReqLoc[i] : " << possibleReqLoc[i] << std::endl;
			
				fullPath = location.getRoot() + possibleReqLoc[i];
				std::cout << BOLD_GREEN << "full path " << fullPath << RESET << std::endl;
				
				fullPath = location.getRoot() + '/' + possibleReqLoc[0]; // for defining whether request is a directory or a file
				if (isDirectory(fullPath))
					_isDir = true;
				else
					_isDir = false;

				return location;
			}
		}
		//else 
		//{
		//	std::cout << "location prefix : " << location.getPrefix() << std::endl;
		//	std::cout << "spiltedDir[i] : " << spiltedDir[i] << std::endl;
		//}
		//else 
		//{
		//	std::cout << "location prefix : " << location.getPrefix() << std::endl;
		//	std::cout << "spiltedDir[i] : " << spiltedDir[i] << std::endl;
		//}
	}
	std::cout << BOLD_RED << "COULD NOT FIND LOCATION BLOCK" << RESET << std::endl;
	throw std::exception(); // this is temporary, will create a error handling mechanism
}

ServerBlock SendData::findServerBlock(std::vector<ServerBlock> &servers, parser &request) // uses the Host header field -> server_name:port -> Host: localhost:8081
{
	std::string host = request.headers["Host"];
	//std::cout << "host : " << host << std::endl;
	//std::cout << "host : " << host << std::endl;
	size_t colon_pos = host.find(':');
	std::string server_name = (colon_pos != std::string::npos) ? host.substr(0, colon_pos) : host;
	std::string port = (colon_pos != std::string::npos) ? host.substr(colon_pos + 1) : ""; // the empty str causes conitional jump on uninitialized value

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		ServerBlock &server = *it;
		// std::cout << "port: " << port << std::endl;
		// std::cout << "host: " << host << std::endl;
		if (findInVector(server.getListen(), stringToInt(port)) && findInVector(server.getServerName(), server_name)) // here port is prioritized over server_name
			return server;
	}
	// this would be fixed later
	std::cout << "\033[1;31m" <<  "returning the first server?, This is a BUG " << "\033[0m" << std::endl;
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
		std::cout << BOLD_GREEN << "FILE : " << file << RESET << std::endl;
		if (read_file(file, request))
			return true;
		i++;
	}
	return false;
}

bool SendData::isCGI(const parser &request, LocationBlock location)
{
	if (request.path.find("/cgi-bin/") != std::string::npos)
	{
		std::string file_extension = '.' +  get_file_extension(request.path);
		std::vector<std::string> allowed_ext = location.getCgiExt();
		bool isAllowed = false;
		for (std::vector<std::string>::iterator it = allowed_ext.begin(); it != allowed_ext.end(); it++)
		{
			/* std::cout << "allowed ext : " << *it << std::endl; */
			if (*it == file_extension)
			{
				isAllowed = true;
				break;
			}
		}
		if (isAllowed)
		{
			/* std::cout << "Extension " << file_extension << " is allowed " << std::endl; */
			return true;
		}
		else
		{
			/* std::cout << "Extension " << file_extension << " is not allowed " << std::endl; */
			return false;
		}
	}
	else
		return false;
}

std::string SendData::sendResponse(int clientSocket, std::vector<ServerBlock> &servers, parser &request, int epollFD)
{
	_isReturn = false;

	ServerBlock current_server = findServerBlock(servers, request);

	if (request.method == "GET")
	{
		try
		{
			LocationBlock location = findLocationBlock(current_server.getLocationVec(), request);
			std::string root = location.getRoot() + request.path;
			/* std::cout << MAGENTA_COLOR << "Root: " << root << std::endl << "Request path:" <<  request.path << RESET << std::endl;
			location.printLocationBlock(); */
			if (location.getReturn().empty())
			{
				/* if (request.path.find("cgi")) // need to check correctly for CGI paths here + if they're in the config file
				{
					CGI cgi(root + request.path, request);
					cgi.setEnv();
					cgi.executeScript();
					cgi.generateResponse();
					cgi.createhtml();
					this->read_file("cgi_output.html", request);
				} */

				if (_isDir)
				{
					bool foundFile = findIndexFile(location.getIndex(), root, request);
					if (!foundFile && location.getAutoindex() == ON) {
						// std::cout << "before AUTOINDEX -> root for this location: " << root << std::endl;
						this->displayDir(root, request.path);
					}
					else if (!foundFile)
						notfound();
				}
				else if (isCGI(request, location))
				{
					std::cout << RED_COLOR << "In CGI" << RESET << std::endl;
					try 
					{
						CGI cgi(root, request);
						cgi.setEnv(current_server);
						cgi.executeScript();
						cgi.generateResponse();
						cgi.createhtml();
						this->read_file("cgi_output.html", request);
					}
					catch (const std::exception &e)
					{
						std::cerr << e.what() << std::endl;
						notfound();
					}
				}
				else
				{
					if (!this->read_file(root, request))
						notfound();
				}
			}
			else
			{
				redirect(location);
				_isReturn = true;
			}
		}
		catch (const std::exception &e)
		{
			// i should here send the right error for invalid locations
			notfound();
		}
	}
	if (request.method == "POST")
	if (request.method == "POST")
	{
		//std::cout << "this body :" << request.body << std::endl;
		saveBodyToFile("../website/upload/" + request.fileName, request);
		_response.status = "HTTP/1.1 200 OK\r\n";
		_response.contentType = "Content-Type: text/html;\r\n";
		_response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
		_response.body += "<body><h1>200 OK</h1><p>file saved</p></body></html>";
		_response.contentLength = "Content-Length: " + intToString(_response.body.size()) + "\r\n";
		_response.body += "<body><h1>200 OK</h1><p>file saved</p></body></html>";
		_response.contentLength = "Content-Length: " + intToString(_response.body.size()) + "\r\n";
	}

	std::string resp;

	if (_isReturn)
		resp = _response.status + _response.location  + _response.contentType + _response.contentLength + "\r\n" + _response.body;
	else
		resp = _response.status + _response.contentType + _response.contentLength + "\r\n" + _response.body;
	

	struct epoll_event client_event;
    client_event.data.fd = clientSocket;
    client_event.events = EPOLLOUT;
	if (epoll_ctl(epollFD, EPOLL_CTL_MOD, clientSocket, &client_event) == -1)
    {
        close(clientSocket);
        throw std::runtime_error("epoll_ctl");
    }
	//std::cout << BLUE_COLOR << "sending response " << RESET << std::endl;
	//std::cout << resp << std::endl;
	return resp;
	

	struct epoll_event client_event;
    client_event.data.fd = clientSocket;
    client_event.events = EPOLLOUT;
	if (epoll_ctl(epollFD, EPOLL_CTL_MOD, clientSocket, &client_event) == -1)
    {
        close(clientSocket);
        throw std::runtime_error("epoll_ctl");
    }
	//std::cout << BLUE_COLOR << "sending response " << RESET << std::endl;
	//std::cout << resp << std::endl;
	return resp;
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

std::vector<std::pair<std::string, std::string> >	listDirectory(const std::string& path) {
	std::vector<std::pair<std::string, std::string> >	elements;
	DIR*	dir = opendir(path.c_str());
	if (dir == NULL) {
		// handle error
		return elements;
	}

	struct dirent* entry;
	std::string	name;
	while ((entry = readdir(dir)) != NULL) {
		name = entry->d_name;
		if (name != "." && (name == ".." || name[0] != '.')) { // not accepting hidden files, except of ".."
			std::string	fullPath = path + '/' + name;
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

std::string escapeHtml(const std::string &input) {

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


void		SendData::displayDir(const std::string& path, const std::string& requestPath) {
	// first pair-element is the element, second one is the full path, but with a '/' at the end for directories
	std::vector<std::pair<std::string, std::string> >	dirElements(listDirectory(path));

	// must embed the dirElements into a html file
	// std::cout << "Elements in directory " << path << ":" << std::endl;
	// std::cout << "Dir/File Name" << std::setw(30) << "Dir/File Path\n";
	// for (size_t i = 0; i < dirElements.size(); ++i) {
	// 	std::cout << dirElements[i].first << std::setw(50) << dirElements[i].second << std::endl;
	// }

	std::ostringstream html;
	html << "<!DOCTYPE html><html><head><title>Index of " << escapeHtml(requestPath) << "</title></head><body>";
	html << "<h1>Index of " << escapeHtml(requestPath) << "</h1>";
	html << "<ul>";

	for (size_t i = 0; i < dirElements.size(); ++i) {
		std::string displayName = dirElements[i].first;
		std::string fullPath = requestPath;
		if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
			fullPath += '/';
		fullPath += displayName;
		// std::cout << "fullpath: " << fullPath << "\n";
		html << "<li><a href=\"" << escapeHtml(fullPath) << "\">" << escapeHtml(displayName) << "</a></li>";
	}

	html << "</ul></body></html>";

	// embed created body inside response struct
	_response.body = html.str();
	// std::cerr << _response.body << std::endl;
	_response.status = "HTTP/1.1 200 OK\r\n";
	_response.contentType = "Content-Type: text/html;\r\n";
	unsigned int content_len = _response.body.size();
	_response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
}
