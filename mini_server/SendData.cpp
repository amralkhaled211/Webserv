#include "SendData.hpp"

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

LocationBlock SendData::findLocationBlock(std::vector<LocationBlock> &locations, parser &request) // this approach for matching might not handle some cases, test required ...
{
	std::vector<std::string> spiltedDir = split(request.path, '/');
	//std::cout << "spiltedDir ;" << spiltedDir[0] << std::endl;
	//std::cout << "spiltedDir size " << spiltedDir.size() << std::endl;
	int i = 0;
	_isDir = true;

	LocationBlock location;
	for (std::vector<LocationBlock>::iterator it = locations.begin(); it != locations.end(); ++it)
	{
		location = *it;
		if (location.getPrefix() == spiltedDir[i])
		{
			std::string fullPath = location.getRoot() + spiltedDir[i];
			std::cout << "full path " << fullPath <<  std::endl;
			while (++i < spiltedDir.size())
			{
				fullPath = fullPath + '/' + spiltedDir[i];
				if (isDirectory(fullPath))
					_isDir = true;
				else
					_isDir = false;
			}
			return location;
		}
	}
	throw std::exception();
}

ServerBlock SendData::findServerBlock(std::vector<ServerBlock> &servers, parser &request) // uses the Host header field -> server_name:port -> Host: localhost:8081
{
	std::string host = request.headers["Host"];
	// std::cout << "-----------------------------------------------------\n";
	// print_map(request.headers);
	// std::cout << request.headers.size() << std::endl;
	// std::cout << "-----------------------------------------------------\n";
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
	std::cout << "NO MATCHING SERVER BLOCK FOUND, TAKING THE FIRST AS DEFAULT" << std::endl;
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
		std::cout << "file : " << file << std::endl;
		if (read_file(file, request))
			return true;
		i++;
	}
	return false;
}

void SendData::sendResponse(int clientSocket, std::vector<ServerBlock> &servers, parser &request)
{
	_isReturn = false;

	ServerBlock current_server = findServerBlock(servers, request);

	if (request.method == "GET")
	{
		try
		{
			LocationBlock location = findLocationBlock(current_server.getLocationVec(), request);
			std::string root = location.getRoot() + request.path;
			if (location.getReturn().empty())
			{
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
	if (request.method == "POST") // this is not an important step, just checking if the Post wrok
	{
		//std::cout << "this body :" << request.body << std::endl;
		saveBodyToFile("../website/upload/amalkhal.txt", request);
		_response.status = "HTTP/1.1 200 OK\r\n";
		_response.contentType = "Content-Type: text/html;\r\n";
		_response.contentLength = "Content-Length: 122\r\n";
		_response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
		_response.body += "<body><h1>200 OK</h1><p>Thank You for login info.</p></body></html>";
	}

	std::string resp;

	if (_isReturn)
		resp = _response.status + _response.location  + _response.contentType + _response.contentLength + "\r\n" + _response.body;
	else
		resp = _response.status + _response.contentType + _response.contentLength + "\r\n" + _response.body;
	const char *resp_cstr = resp.c_str();
	size_t resp_length = resp.size();
	send(clientSocket, resp_cstr, resp_length, 0);
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
	std::cout << "Elements in directory " << path << ":" << std::endl;
	std::cout << "Dir/File Name" << std::setw(30) << "Dir/File Path\n";
	for (size_t i = 0; i < dirElements.size(); ++i) {
		std::cout << dirElements[i].first << std::setw(50) << dirElements[i].second << std::endl;
	}

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
		std::cout << "fullpath: " << fullPath << "\n";
		html << "<li><a href=\"" << escapeHtml(fullPath) << "\">" << escapeHtml(displayName) << "</a></li>";
	}

	html << "</ul></body></html>";

	// embed created body inside response struct
	_response.body = html.str();
	std::cerr << _response.body << std::endl;
	_response.status = "HTTP/1.1 200 OK\r\n";
	_response.contentType = "Content-Type: text/html;\r\n";
	unsigned int content_len = _response.body.size();
	_response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
}
