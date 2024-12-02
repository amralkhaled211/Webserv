#include "Client.hpp"

Client::Client()
{
	isAllRecieved = false;
	_isChunked = false;
	_headersIsChunked = false;
	_sentHeader = false;
	_newLineChecked = false;
	request.statusError = 0;
}


void Client::setBuffer(const std::string& buffer)
{
    _buffer = buffer;
}

void Client::setResponseBuffer(std::string resBuffer)
{
	this->_responseBuffer = resBuffer;
}

parser &Client::getRequest()
{
	return request;
}

bool Client::parse_first_line()
{
	size_t start = 0;
	size_t end = this->_buffer.find(' ', start);
	request.method = deleteSpaces(this->_buffer.substr(start, end - start));
	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
	{
	    std::cerr << "Error: bad request method" << std::endl;
	    request.statusError = 400;
	    return false;
	}
	start = end + 1;
	end = this->_buffer.find(' ', start);
	request.path = deleteSpaces (this->_buffer.substr(start, end - start));
	if (request.path.empty())
	{
		std::cerr << "Error: bad request path" << std::endl;
		request.statusError = 400;
		return false;
	}
	start = end + 1;
	end = this->_buffer.find("\r\n", start);
	request.version = deleteSpaces(this->_buffer.substr(start, end - start));
	if (request.version != "HTTP/1.1") // if it does not exist we should serve default page. (HTTP/1.1 only accepted)
	{
		std::cerr << "Error: bad request version" << std::endl;
		request.statusError = 400;
		return false;
	}
	return true;
}

void Client::parseHeaders(std::string &Buffer)
{
	std::istringstream headerStream(Buffer);
    std::string line;
    while (std::getline(headerStream, line) && !line.empty())
	{
        size_t delimiterPos = line.find(":");
        if (delimiterPos != std::string::npos)
		{
            std::string headerName = line.substr(0, delimiterPos);
            std::string headerValue = line.substr(delimiterPos + 1);
            request.headers[headerName] = deleteSpaces(headerValue);
        }
    }
}

bool Client::parse_body(std::string& body)
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
	if (this->request.body.find(endBoundary) != std::string::npos)
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

bool Client::HandlChunk()
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
		if (this->request.body.find(endBoundary) != std::string::npos)
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

bool Client::headersValidate(std::string &buffer, std::string method)
{
	if (method == "GET")
	{
		std::istringstream headerStream(this->_buffer);
		std::string line;
		while (std::getline(headerStream, line))
    	{
    	    if (line == "\r" && request.headers.find("Host") != request.headers.end() && !request.headers["Host"].empty()) // Check for the end of headers
			{
				std::cout << "end of headers" << std::endl;
				request.statusError = 0;
				return true;
			}
			else if (line == "\r" && request.headers.find("Host") == request.headers.end())
			{
				std::cerr << "Error: Bad request 400" << std::endl;
				request.statusError = 400;
				return true;
			}
    	}
	}
	if (method == "POST")
	{
		std::istringstream headerStream(this->_buffer);
		std::string line;
		while (std::getline(headerStream, line))
    	{
    	    if (line == "\r" && request.headers.find("Host") != request.headers.end() && !request.headers["Host"].empty() &&
					request.headers.find("Content-Length") != request.headers.end() && !request.headers["Content-Length"].empty() &&
					request.headers.find("Content-Type") != request.headers.end() && !request.headers["Content-Type"].empty()) // Check for the end of headers
			{
				std::cout << "end of headers" << std::endl;
				request.statusError = 0;
				return true;
			}
			else if (line == "\r" && request.headers.find("Host") == request.headers.end() &&
						request.headers.find("Content-Length") == request.headers.end() &&
						request.headers.find("Content-Type") == request.headers.end())
			{
				std::cerr << "Error: Bad request 400" << std::endl;
				request.statusError = 400;
				return true;
			}
    	}
	}
	return false;
}


bool Client::handlingBody(std::string &body)
{
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
	
	return false;
}

bool Client::bodyValidate(std::string &Buffer)
{

	std::istringstream headerStream(Buffer);
	std::string line;
	// std::cout << "request.body.size() " << request.body.size() << std::endl;
	if (request.body.empty() && !_newLineChecked)
	{
		std::getline(headerStream, line);
		std::cout << "skip the new line :"  << std::endl;
		if (line == "\r")
		{
			_newLineChecked = true;
			return false;	
		}
	}
	request.body = Buffer;
	std::cout << "request.body=====> ;" << request.body << std::endl;
	while (std::getline(headerStream, line))
    {
        if (line == "\r" && !request.body.empty()) // Check for the end of headers
		{
			std::cout << "end of body" << std::endl;
			request.statusError = 0;
			return true;
		}
		else if ((line == "\r" && request.body.empty()))
		{
			std::cerr << "Error: Bad request 400" << std::endl;
			request.statusError = 400;
			return true;
		}
    }
	return false; // this mean we still expecting more chunks
}

bool Client::parseHeadersAndBody()
{
	if (_isChunked )
	{
		if (HandlChunk())
			return true;
	}
	else if (request.method == "POST")
	{
		size_t headerEndPos = this->_buffer.find("\r\n\r\n");
		if (headerEndPos == std::string::npos || _newLineChecked) // this would be an indcation that the headers would be on chunks 
		{
			std::cout << "headerEndPos not found" << std::endl;
			std::cout << "this-> buffer =====> : " << this->_buffer << std::endl;
			parseHeaders(this->_buffer);
			std::cout << "content length : " << request.headers["Content-Length"] << std::endl;
			if (headersValidate(this->_buffer, request.method)) // if this true that means we have the headers and now we ganna do the same thing for the body
			{
				std::cout << "i am inside the header vaildatio :: " << std::endl;
				if (request.statusError == 0) // this mean we are expecting a body if we dont have on then its not valid and we send a message
				{
					std::cout << "i am ready for the body : " << std::endl;
					if (bodyValidate(this->_buffer))
						return true;
					else
				    {
						return false;
					}
				}
				else
				{
					return true;
				}
			}
			else
			{
				_headersIsChunked = true;
				return false;
			}
		}
		else 
		{
			std::cout << "i am going to the else " << std::endl;
			std::string body = this->_buffer.substr(headerEndPos + 4);
			std::string headerSection = this->_buffer.substr(0, headerEndPos);
			parseHeaders(headerSection);
			if (handlingBody(body))
			{
				std::cout << "i am going to the else true" << std::endl;
				return true;
			}
			else
			{
				std::cout << "i am going to the else false" << std::endl;
				return false;
			}
		}
	}
	else /// this will be for the GET request
	{
		std::cout << "coming to the get validation " << std::endl;
		parseHeaders(this->_buffer);
		if (headersValidate(this->_buffer, request.method))
			return true; // this would mean the headers are not chunked 
		else 
			_headersIsChunked = true;
	}
	std::cout << "we throwing this ::: " << std::endl;	
	return false;
}


void Client::allRecieved()
{
	if (!_isChunked && !_headersIsChunked)
	{
		if (!parse_first_line())
		{
			isAllRecieved = true;
			return;
		}
	}
	parseQueryString();
	if (parseHeadersAndBody())
	{
		// this would reset values
		_isChunked = false;
		_headersIsChunked = false;
		_bytesRead = 0;
		_targetBytes = 0;
		_newLineChecked = false;


		isAllRecieved = true;
		if (request.body.size() > 0)
		{
			saveBodyToFile();
		}
		return;
	}
	isAllRecieved = false;
	if (request.body.size() > 0)
	{
		std::cout << "i Am reading chuncked file " << std::endl;
		saveBodyToFile();
	}
}


void Client::saveBodyToFile()
{
	std::string filePath;
	if (!request.fileName.empty())
	{
		filePath = "../website/upload/" + request.fileName;
	}
	else 
		filePath = "../website/upload/data.txt";

	//check if file already exists
    std::ifstream infile(filePath.c_str());
    if (infile.good())
    {
		//we might want to handle this differently like an error page or something
        std::cout << "File already exists: " << filePath << std::endl;
	}
	infile.close();

    std::ofstream outFile(filePath.c_str(), std::ios::binary | std::ios::app);
    if (outFile.is_open())
    {
        outFile.write(request.body.c_str(), request.body.size());
        outFile.close();
    }
    else
    {
        // Handle error opening file
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
    }
	request.body.clear();
}


void Client::parseQueryString()
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