#include "server.hpp"

void Server::parse_first_line()
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

void Server::parseHeaders()
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
void Server::parseRequest()
{
	parse_first_line();
	parseHeaders();
}