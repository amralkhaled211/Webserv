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
	std::cout << BOLD_GREEN << "recieved request Buffer: \n" << RESET;
	std::cerr << _buffer << std::endl;
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

void RequestHandler::parse_body(std::string& body)
{
	std::istringstream stream(body);
	std::string line;
	std::getline(stream, line);
	//std::cout << "first line "  << line << std::endl;
	while (std::getline(stream, line))
	{
		//std::cout << GREEN_COLOR << "body Line :" << RESET_COLOR << line << std::endl;
		break;
	}
}


void RequestHandler::parseHeaders(std::string &Buffer)
{
	std::istringstream headerStream(Buffer);
    std::string line;
    while (std::getline(headerStream, line) && !line.empty()) {
        size_t delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos) {
            std::string headerName = line.substr(0, delimiterPos);
            std::string headerValue = line.substr(delimiterPos + 2);
            request.headers[headerName] = headerValue;
        }
        //std::cout << GREEN_COLOR << "Headers Line :" << RESET_COLOR << line << std::endl;
    }
}

void RequestHandler::parseHeadersAndBody()
{

	//std::cout << RED << "Buffer :" << this->_buffer << RESET << std::endl;
	if (request.method == "POST")
	{
		size_t headerEndPos = this->_buffer.find("\r\n\r\n");
		std::string body = this->_buffer.substr(headerEndPos + 4);
		// request.body = body;
		//std::cout << "Body : " << body << std::endl;
		parse_body(body);
	}

	std::string headerSection = this->_buffer;//.substr(0, this->_buffer.find("\r\n\r\n"));
	std::istringstream headerStream(headerSection);
	std::string line;
	while (std::getline(headerStream, line) && !line.empty())
	{
		size_t delimiterPos = line.find(": ");
		if (delimiterPos != std::string::npos)
		{
			std::string headerName = line.substr(0, delimiterPos);
			std::string headerValue = line.substr(delimiterPos + 2);
			request.headers[headerName] = headerValue;
		}
		//std::cout << GREEN_COLOR << "Headrs Line :" << RESET_COLOR << line << std::endl;
	}
}


void RequestHandler::parseRequest()
{
	parse_first_line();
	parseHeadersAndBody();
}


// void RequestHandler::parseRequest() {
//     parse_first_line();
//     parseHeaders();

//     if (request.method == "POST") {
//         auto it = request.headers.find("Content-Type");
//         if (it != request.headers.end() && it->second.find("multipart/form-data") != std::string::npos) {
//             std::string boundary = "--" + it->second.substr(it->second.find("boundary=") + 9);
//             std::string body;
//             std::istringstream stream(this->_buffer);
//             std::getline(stream, body, '\0'); // Read the entire body

//             size_t pos = 0;
//             while ((pos = body.find(boundary, pos)) != std::string::npos) {
//                 pos += boundary.length();
//                 size_t end = body.find(boundary, pos);
//                 std::string part = body.substr(pos, end - pos);
//                 pos = end;

//                 // Process each part
//                 std::istringstream partStream(part);
//                 std::string partLine;
//                 std::unordered_map<std::string, std::string> partHeaders;
//                 while (std::getline(partStream, partLine) && partLine != "\r\n") {
//                     size_t dilm = partLine.find(":");
//                     std::string key = deleteSpaces(partLine.substr(0, dilm));
//                     std::string value = deleteSpaces(partLine.substr(dilm + 1, partLine.length()));
//                     partHeaders[key] = value;
//                 }

//                 std::string content;
//                 std::getline(partStream, content, '\0'); // Read the content of the part
//                 // Store or process the content as needed
//             }
//         } else {
//             std::string line;
//             std::istringstream stream(this->_buffer);
//             while (std::getline(stream, line)) {
//                 request.body += line;
//             }
//         }
//     }
// }