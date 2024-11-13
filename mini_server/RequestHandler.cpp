#include "RequestHandler.hpp"

void RequestHandler::receiveData(int clientSocket)
{
	char Buffer[60000] = {0};
	int bytesReceived = recv(clientSocket, Buffer, sizeof(Buffer), 0);
	if (bytesReceived < 0)
		throw std::runtime_error("Receiving failed");
	this->_buffer.assign(Buffer, bytesReceived); // this would copy only the data that was received
	/* std::cout << BOLD_GREEN << "recieved request Buffer: \n" << RESET;
	std::cerr << BOLD_WHITE << _buffer << RESET << std::endl; */
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

void RequestHandler::parseHeaders(std::string &Buffer)
{
	std::istringstream headerStream(Buffer);
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
    }
}

bool RequestHandler::parse_body(std::string& body)
{
	std::istringstream stream(body);
	std::string line;
	std::getline(stream, line); // this ganna be the first _boundary
	std::getline(stream, line); // this ganna be the scond Content-Disposition and file name
	request.fileName = line.substr(line.find("filename=") + 9);
	if (!request.fileName.empty() && request.fileName[0] == '"' && request.fileName[request.fileName.size() - 2] == '"')
		request.fileName = request.fileName.substr(1, request.fileName.size() - 3);
	std::getline(stream, line); // this ganna be the third Content-Type:


	std::string endBoundary = _boundary + "--\r";
	size_t headerEndPos = this->_buffer.find("\r\n\r\n");
	request.body = this->_buffer.substr(headerEndPos + 4);
	if (this->_buffer.find(endBoundary) != std::string::npos)
	{
		std::size_t pos = request.body.rfind(endBoundary);
        	if (pos != std::string::npos)
        	    request.body.resize(pos);
		if (!request.body.empty() && request.body[request.body.size() - 1] == '\n')
                request.body.erase(request.body.size() - 2);
		std::cout << "end of body  dont read anymore " << std::endl;
		return true;
	}
	return false;
}

bool RequestHandler::HandlChunk()
{
	if (_bytesRead == 0 && !_boundary.empty()) //this is if the begining of boundry_body is in another chunk
	{
		_bytesRead += this->_buffer.size(); 
		if (parse_body(this->_buffer))
			return true;
	}
	else if (_bytesRead != 0 &&!_boundary.empty()) // this if middle of boundry_body is in another chunk
	{
		std::istringstream stream(this->_buffer);
		std::string line;
		_bytesRead += this->_buffer.size();
		std::string endBoundary = _boundary;
		endBoundary = endBoundary.substr(0, endBoundary.size() - 1) + "--\r"; // i have to tirm the /r first
		request.body += this->_buffer;
		if (this->_buffer.find(endBoundary) != std::string::npos)
		{
			std::size_t pos = request.body.rfind(endBoundary);
        	if (pos != std::string::npos)
        	    request.body.resize(pos);
			if (!request.body.empty() && request.body[request.body.size() - 1] == '\n')
                request.body.erase(request.body.size() - 2);
			std::cout << "end of body  dont read anymore " << std::endl;
			return true;
		}
	}
	else // this would be for chunks that has no boundry 
	{
		request.body += this->_buffer;
		_bytesRead += this->_buffer.size(); 
		if (_bytesRead == _targetBytes)
			return true;
	}
	return false;
}

bool RequestHandler::parseHeadersAndBody()
{
	if (_isChunked)
	{
		if (HandlChunk())
			return true;
	}
	else if (request.method == "POST")
	{
		size_t headerEndPos = this->_buffer.find("\r\n\r\n");
		std::string body = this->_buffer.substr(headerEndPos + 4);
		std::string headerSection = this->_buffer.substr(0, headerEndPos);
		parseHeaders(headerSection);

		std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
		if (it != request.headers.end() && it->second.find("multipart/form-data") != std::string::npos)
		{
			_boundary = "--" + it->second.substr(it->second.find("boundary=") + 9);
		}

		if (body.size() > 0)// this is if the body comes with the headers in one chunk
		{
			if (!_boundary.empty())//that would mean we would have to upload a file 
			{
				if (parse_body(body))
					return true;
			}
			else
				request.body = body;
		}
		_bytesRead = body.size();
		_targetBytes = stringToSizeT(request.headers["Content-Length"]);
		if (_bytesRead < _targetBytes)
		{
			_isChunked = true;
			return false;
		}
		else if (_bytesRead == _targetBytes)
			return true;
	}
	else
	{
		parseHeaders(this->_buffer);
		return true;
	}
	return false;
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

bool RequestHandler::parseRequest()
{
	if (!_isChunked)
	{
		parse_first_line();
	}
	parseQueryString();
	if (parseHeadersAndBody())
	{
		//std::cout << "ready to send  " << std::endl;
		_isChunked = false;
		_bytesRead = 0;
		size_t _targetBytes = 0;
		//std::cout << request.body << std::endl;
		return true;
	}
	return false;
}
RequestHandler::RequestHandler()
{
	_isChunked = false;
	//std::cout << "just called this object " << std::endl;
}