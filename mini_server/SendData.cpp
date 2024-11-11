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

bool SendData::read_file(std::string const &path, parser &request)
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

LocationBlock SendData::findLocationBlock(std::vector<LocationBlock> &locations, parser &request)
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
		//else 
		//{
		//	std::cout << "location prefix : " << location.getPrefix() << std::endl;
		//	std::cout << "spiltedDir[i] : " << spiltedDir[i] << std::endl;
		//}
	}
	throw std::exception();
}

ServerBlock SendData::findServerBlock(std::vector<ServerBlock> &servers, parser &request)
{
	std::string host = request.headers["Host"];
	//std::cout << "host : " << host << std::endl;
	size_t colon_pos = host.find(':');
	std::string server_name = (colon_pos != std::string::npos) ? host.substr(0, colon_pos) : host;
	std::string port = (colon_pos != std::string::npos) ? host.substr(colon_pos + 1) : "";

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		ServerBlock &server = *it;
		if (findInVector(server.getListen(), stringToInt(port)) && findInVector(server.getServerName(), server_name))
			return server;
	}
	// this would be fixed later
	std::cout << "\033[1;31m" <<  "returning the first server?, This is a BUG " << "\033[0m" << std::endl;
	return servers[0]; // return default
}

void SendData::redirect(LocationBlock& location)
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

bool SendData::findIndexFile(const std::vector<std::string> &files, std::string &root, parser &request)
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
				if (_isDir)
				{
					if (!findIndexFile(location.getIndex(), root, request))
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
	{
		//should make a flag her if i want to save the file or save user info database
		saveBodyToFile("/home/amalkhal/Webserv/website/upload/" + request.fileName, request);

		//this is hard coded for now
		_response.status = "HTTP/1.1 200 OK\r\n";
		_response.contentType = "Content-Type: text/html;\r\n";
		_response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
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