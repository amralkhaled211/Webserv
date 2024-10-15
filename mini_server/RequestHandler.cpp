#include "RequestHandler.hpp"
#include "CGI.hpp"

// this is handling the request part
void RequestHandler::receiveData(int clientSocket)
{
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0)
    {
        close(clientSocket);
        throw std::runtime_error("Receiving failed");
    }
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
		//std::cout << "the body :" << request.body << std::endl;
	}
	// std::cout << "this buffer" << this->buffer << std::endl;
}

void RequestHandler::parseQueryString()
{
	size_t pos = request.path.find('?');
	if (pos != std::string::npos)
	{
		request.queryString = request.path.substr(pos + 1);
		request.path = request.path.substr(0, pos);
	}
	else
		request.queryString = "";
}

void RequestHandler::parseRequest()
{
	parse_first_line();
	parseQueryString();
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

void RequestHandler::read_file(std::string const &path)
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

/* bool is_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return false; // Assume blocking if we can't get the flags
    }
    return (flags & O_NONBLOCK) != 0;
} */

void RequestHandler::sendResponse(int clientSocket)
{
	std::string root = "/home/aszabo/Docs/webserv/website";// this would be changed 

	if (request.method == "GET")
	{
		/* std::cout << "Request path " << request.path << std::endl; */
		if (request.path == "/")// this if the whole path is not given, so i would give a default path file 
			request.path = "/index.html";
		if (request.path.find("cgi") != std::string::npos)
		{
			//print_map(request.headers);
			std::cout << "Executing CGI now" << std::endl;
			std::cout << "CGI script path " << root + request.path << std::endl;
			CGI cgi(root + request.path, request);
			//print_map(request.headers);
			cgi.setEnv();		//we will need env variables from config file here later
			//cgi.printEnv();
			cgi.executeScript();
			cgi.generateResponse();
			response = cgi.getResponse();
			/* std::cout << "Response status: " << response.status << std::endl;
			std::cout << "Response content type: " << response.contentType << std::endl;
			std::cout << "Response content length: " << response.contentLength << std::endl;
			std::cout << "Response body: " << response.body << std::endl; */
		}
		else
			this->read_file(root + request.path);
	}
	if (request.method == "POST") // this is not an important step, just checking if the Post wrok
	{
		if (request.path.find(".cgi") != std::string::npos)
		{
			CGI cgi(root + request.path, request);
			cgi.setEnv();//we will need env variables from config file here later
			cgi.executeScript();
			cgi.generateResponse();
			response = cgi.getResponse();
		}
		else
		{
			std::cout << "the body :" << request.body << std::endl;
			response.status = "HTTP/1.1 200 OK\r\n";
			response.contentType = "Content-Type: text/html;\r\n";
			response.contentLength = "Content-Length: 122\r\n";
			response.body = "<!DOCTYPE html><html><head><title>200 OK</title></head>";
			response.body += "<body><h1>200 OK</h1><p>Thank You for login info.</p></body></html>";
		}
	}
	std::string resp =  response.status + response.contentType + response.contentLength + "\r\n" + response.body;
	std::cout << "Response: " << resp << std::endl;

	const char* resp_cstr = resp.c_str();
	/* std::cout << "Response c_str: " << resp.c_str() << std::endl; */

    size_t resp_length = resp.size();
	/* std::cout << "Response length: " << resp_length << std::endl; */

	//make_socket_non_blocking(clientSocket);
    //send(clientSocket, resp_cstr, resp_length, 0);
	size_t totalBytesSent = 0;

    while (totalBytesSent < resp_length)
    {
        ssize_t bytesSent = send(clientSocket, resp_cstr + totalBytesSent, resp_length - totalBytesSent, 0);
        if (bytesSent == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // If the socket is non-blocking and the send would block, continue
                continue;
            }
            else
            {
                perror("send");
                close(clientSocket);
                return;
            }
        }
        totalBytesSent += bytesSent;
    }

	std::cout << "Sent response" << std::endl;
}