#include "RequestHandler.hpp"




// void RequestHandler::receiveData(int clientSocket)
// {
//     const size_t chunkSize = 1024;
//     std::vector<char> buffer;
//     char tempBuffer[chunkSize];
//     int bytesReceived;

//     while (true)
//     {
//         bytesReceived = recv(clientSocket, tempBuffer, chunkSize, 0);
//         if (bytesReceived < 0)
//         {
//             std::cerr << "Receiving failed: " << strerror(errno) << std::endl;
//             throw std::runtime_error("Receiving failed");
//         }
//         if (bytesReceived == 0)
//             break; // Connection closed

//         buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesReceived);
//     }

//     this->_buffer.assign(buffer.begin(), buffer.end()); // Copy all received data to _buffer
// }


void RequestHandler::receiveData(int clientSocket)
{
	char Buffer[1024] = {0};
	int bytesReceived = recv(clientSocket, Buffer, sizeof(Buffer), 0);
	if (bytesReceived < 0)
		throw std::runtime_error("Receiving failed");
	// 	this->buffer = buffer; this would copy the whole buffer this might cause storing carbege data if the buffer is not full
	this->_buffer.assign(Buffer, bytesReceived); // this would copy only the data that was received
}

parser& RequestHandler::getRequest()
{
    return request;
}

void RequestHandler::parse_first_line()
{
	size_t start = 0;
	size_t end = this->_buffer.find(' ', start);
	request.method = this->_buffer.substr(start, end - start);
	start = end + 1;
	end = this->_buffer.find(' ', start);
	request.path = this->_buffer.substr(start, end - start);
	start = end + 1;
	end = this->_buffer.find("\r\n", start);
	request.version = this->_buffer.substr(start, end - start);
}

void RequestHandler::parseHeaders()
{
	std::string line;
	std::istringstream stream(this->_buffer);
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
	}
	// std::cout << "this _buffer" << this->buffer << std::endl;
}
void RequestHandler::parseRequest()
{
	parse_first_line();
	parseHeaders();
}

