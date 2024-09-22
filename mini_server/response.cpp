#include "server.hpp"

void Server::notfound()
{
	response.status = "HTTP/1.1 404 Not Found\r\n";
	response.contentType = "Content-Type: text/html;\r\n";
	response.contentLength = "Content-Length: 155\r\n";
	response.body += "<!DOCTYPE html><html><head><title>404 Not Found</title></head>";
	response.body += "<body><h1>404 Not Found</h1><p>The page you are looking for does not exist.</p></body></html>";
}

void Server::read_file(std::string const &path)
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
	}
	else
	{
		notfound();
	}
}

void Server::sendResponse()
{
	std::string root = "/home/amalkhal/mySite";

	if (request.method == "GET")
	{
		// i still need to check if the the user did not pass any path
		// i have to check with the nginx server how it handles the path
		if (request.path == "/")
		{
			request.path = "/index.html";
		}
		read_file(root + request.path);
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
	std::string resp =  response.status + response.contentType + response.contentLength + "\r\n" + response.body;
	const char* resp_cstr = resp.c_str();
    size_t resp_length = resp.size();
    send(clientSocket, resp_cstr, resp_length, 0);
}