#include "RequestHandler.hpp"

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
	std::cout << "this buffer" << this->buffer << std::endl;
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


ServerBlock RequestHandler::findServerBlock(std::vector<ServerBlock>& servers)
{
    std::string host = request.headers["Host"];
    size_t colon_pos = host.find(':');
    std::string server_name = (colon_pos != std::string::npos) ? host.substr(0, colon_pos) : host;
    std::string port = (colon_pos != std::string::npos) ? host.substr(colon_pos + 1) : "";

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it !=  servers.end(); ++it)
	{
		ServerBlock& server = *it;
		if (findInVector(server.getListen(), stringToInt(port)) && findInVector(server.getServerName(), server_name))
			return server;
	}
	// this would be fixed later 
	std::cout << "returning the first server " << std::endl; 
	return servers[0];// return default
}

void RequestHandler::sendResponse(int clientSocket, std::vector<ServerBlock>& servers)
{

	ServerBlock current_server = findServerBlock(servers);

	//std::cout << "current_server port : " << current_server.getListen() << std::endl;



	std::string root = "/home/amalkhal/Webserv/website";// this would be changed 

	if (request.method == "GET")
	{
		std::cout << "path :"  << request.path << std::endl; 
		if (request.path == "/")// this if the whole path is not given, so i would give a default path file 
			request.path = "/index.html";
		this->read_file(root + request.path);
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