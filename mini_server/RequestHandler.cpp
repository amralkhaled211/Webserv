#include "RequestHandler.hpp"


// #include <iostream>
// #include <vector>
// #include <stdexcept>
// #include <sys/socket.h> // For socket functions

// void receiveLargeData(int clientSocket) {
//     const size_t bufferSize = 1024;
//     char buffer[bufferSize] = {0};
//     std::vector<char> receivedData;
//     int bytesReceived = 0;

//     while (true) {
//         bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
//         if (bytesReceived < 0) {
//             throw std::runtime_error("Receiving failed");
//         } else if (bytesReceived == 0) {
//             // No more data to receive
//             break;
//         }
//         receivedData.insert(receivedData.end(), buffer, buffer + bytesReceived);
//     }

//     // Now receivedData contains all the data received
//     // You can process it as needed
//     std::cout << "Total bytes received: " << receivedData.size() << std::endl;
// }


// this is handling the request part
void RequestHandler::receiveData(int clientSocket)
{
	char buffer[1024] = {0};
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesReceived < 0)
		throw std::runtime_error("Receiving failed");
	// 	this->buffer = buffer; this would copy the whole buffer this might cause storing carbege data if the buffer is not full
	this->buffer.assign(buffer, bytesReceived); // this would copy only the data that was received
}

void RequestHandler::parse_first_line()
{
	size_t start = 0;
	size_t end = this->buffer.find(' ', start);
	request.method = this->buffer.substr(start, end - start);
	start = end + 1;
	end = this->buffer.find(' ', start);
	request.path = this->buffer.substr(start, end - start);
	start = end + 1;
	end = this->buffer.find("\r\n", start);
	request.version = this->buffer.substr(start, end - start);
}

void RequestHandler::parseHeaders()
{
	std::string line;
	std::istringstream stream(this->buffer);
	std::getline(stream, line);
	while (std::getline(stream, line))
	{
		size_t dilm = line.find(":");
		std::string key = deleteSpaces(line.substr(0, dilm));
		std::string value = deleteSpaces(line.substr(dilm + 1, line.length()));
		request.headers[key] = value;
	}
	if (request.method == "POST") // TODO : i would need to parse the body in more advanced way
	{
		std::getline(stream, line);
		request.body = line;
		// std::cout << "the body :" << request.body << std::endl;
	}
	// std::cout << "this buffer" << this->buffer << std::endl;
}
void RequestHandler::parseRequest()
{
	parse_first_line();
	parseHeaders();
}

////// this is response part

void RequestHandler::notfound()
{
	response.status = "HTTP/1.1 404 Not Found\r\n";
	response.contentType = "Content-Type: text/html;\r\n";
	response.contentLength = "Content-Length: 155\r\n";
	response.body += "<!DOCTYPE html><html><head><title>404 Not Found</title></head>";
	response.body += "<body><h1>404 Not Found</h1><p>The page you are looking for does not exist.</p></body></html>";
}

bool RequestHandler::read_file(std::string const &path)
{
	std::string file_extension = get_file_extension(request.path);
	std::ifstream file(path.c_str());
	if (file.is_open())
	{
		response.body.clear();
		std::string line;
		while (std::getline(file, line))
		{
			response.body += line + "\n";
		}
		response.status = "HTTP/1.1 200 OK\r\n";
		response.contentType = "Content-Type: " + mimeTypesMap_G[file_extension] + ";" + "\r\n";
		unsigned int content_len = response.body.size();
		response.contentLength = "Content-Length: " + intToString(content_len) + "\r\n";
		file.close();
		return true;
	}
	return false;
}

ServerBlock RequestHandler::findServerBlock(std::vector<ServerBlock> &servers)
{
	std::string host = request.headers["Host"];
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
	std::cout << "returning the first server " << std::endl;
	return servers[0]; // return default
}
bool RequestHandler::findIndexFile(const std::vector<std::string> &files, std::string &root)
{
	size_t i = 0;

	while (i < files.size())
	{
		std::string file = root + '/' + files[i];
		std::cout << "file : " << file << std::endl;
		if (read_file(file))
			return true;
		i++;
	}
	return false;
}

LocationBlock RequestHandler::findLocationBlock(std::vector<LocationBlock> &locations)
{
	std::vector<std::string> spiltedDir = split(request.path, '/');
	std::cout << "spiltedDir ;" << spiltedDir[0] << std::endl;
	std::cout << "spiltedDir size " << spiltedDir.size() << std::endl;
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

void RequestHandler::redirect(LocationBlock& location)
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
		code = "301";
		url = returnVec[0];
	}
	response.status = "HTTP/1.1 " + code + " " + redir.CodeToMessage[code] + "\r\n";
	response.location = "Location: " + url + "\r\n";
	response.contentType = "Content-Type: text/html;\r\n";
	response.contentLength = "Content-Length: 0\r\n";
	response.body = "";
}

void RequestHandler::sendResponse(int clientSocket, std::vector<ServerBlock> &servers)
{
	_isReturn = false;

	ServerBlock current_server = findServerBlock(servers);

	if (request.method == "GET")
	{
		try
		{
			LocationBlock location = findLocationBlock(current_server.getLocationVec());
			std::string root = location.getRoot() + request.path;
			if (location.getReturn().empty())
			{
				if (_isDir)
				{
					if (!findIndexFile(location.getIndex(), root))
						notfound();
				}
				else
				{
					if (!this->read_file(root))
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
		std::cout << "the body :" << request.body << std::endl;
		response.status = "HTTP/1.1 200 OK\r\n";
		response.contentType = "Content-Type: text/html;\r\n";
		response.contentLength = "Content-Length: 122\r\n";
		response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
		response.body += "<body><h1>200 OK</h1><p>Thank You for login info.</p></body></html>";
	}

	std::string resp;

	if (_isReturn)
		resp = response.status + response.location  + response.contentType + response.contentLength + "\r\n" + response.body;
	else
		resp = response.status + response.contentType + response.contentLength + "\r\n" + response.body;
	const char *resp_cstr = resp.c_str();
	size_t resp_length = resp.size();
	send(clientSocket, resp_cstr, resp_length, 0);
}