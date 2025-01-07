#include "Client.hpp"

Client::Client()
{
	isAllRecieved = false;
	_isChunked = false;
	_headersIsChunked = false;
	_sentHeader = false;
	_newLineChecked = false;
	_chunkLengthRecieved = false;
	request.statusError = 0;
	_chunkLengthValue = 0;
	_bytesRead = 0;
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


bool Client::parseHeadersAndBody()
{
	//std::cerr << BOLD_GREEN << "this buffer"<< this->_buffer << RESET << std::endl;
	if (_isChunked)
	{
		// std::cout << "i am inside the chunked " << std::endl;
		if (HandlChunk())
			return true;
	}
	else if (request.method == "POST")
	{
		size_t headerEndPos = this->_buffer.find("\r\n\r\n");
		if (headerEndPos == std::string::npos) // this would be an indcation that the headers would be on chunks 
		{
			// std::cout << "headerEndPos not found" << std::endl;
			if (parseHeaders(this->_buffer))	
				return true;
			if (headersValidate(request.method) || _newLineChecked) // if this true that means we have the headers and now we ganna do the same thing for the body
			{
				if (request.statusError == 400)
					return true;
				if (findMaxBodySize())
					return true;
				// std::cout << "i am inside the header vaildatio :: " << std::endl;
				if (request.statusError == 0) // this mean we are expecting a body if we dont have on then its not valid and we send a message
				{
					// std::cout << "i am ready for the body : " << std::endl;
					if (bodyValidate(this->_buffer)) // i could use later here handling body function
						return true;
					else
				    {
						return false; 
					}
				}
				else
				{
					return true; /// this would mean i am receiving an errot status 400
				}
			}
			else
			{
				// std::cout << "i am stuck hrer " << std::endl;
				_headersIsChunked = true;
				return false;
			}
		}
		else 
		{
			// std::cout << "i am going to the else " << std::endl;
			std::string body = this->_buffer.substr(headerEndPos + 4);
			std::string headerSection = this->_buffer.substr(0, headerEndPos);
			parseHeaders(headerSection);
			if (findMaxBodySize())
				return true;
			if (handlingBody(body))
			{
				// std::cout << "giving true means end of body" << std::endl;
				return true;
			}
		}
	}
	else
	{
		//std::cout << "coming to the get validation " << std::endl;
		if (parseHeaders(this->_buffer))
			return true;
		if (headersValidate(request.method))
			return true; // this would mean the headers are not chunked 
		else 
			_headersIsChunked = true;
	}
	// std::cout << "we throwing this ::: " << std::endl;
	return false;
}



void Client::saveBodyToFile()
{
	std::string filePath;
	if (!request.fileName.empty())
	{
		filePath = "../www/website/upload/" + request.fileName;
	}
	else 
		filePath = "../www/website/upload/data.txt";

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
		request.statusError = 503;
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
bool Client::parse_first_line()
{
	size_t start = 0;
	size_t end = this->_buffer.find(' ', start);
	request.method = deleteSpaces(this->_buffer.substr(start, end - start));
	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
	{
	    //std::cerr << "Error: bad request method" << std::endl;
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

bool Client::parseHeaders(std::string &Buffer)
{
	std::istringstream headerStream(Buffer);
    std::string line;
    while (std::getline(headerStream, line) && !line.empty())
	{
        size_t delimiterPos = line.find(":");
        if (delimiterPos != std::string::npos)
		{
            std::string headerName = line.substr(0, delimiterPos); // here i woud check for duplicate
			if (request.headers.find(headerName) != request.headers.end() ||
				headerName.find(' ') != std::string::npos ||
				headerName.find('\t') != std::string::npos)
			{
				std::cerr << "Error: duplicate header or spaces or tabs" << std::endl;
				request.statusError = 400;
				return true;
			}
            std::string headerValue = line.substr(delimiterPos + 1);
			headerValue.erase(headerValue.find_last_not_of(" \n\r\t") + 1);
            request.headers[headerName] = deleteSpaces(headerValue);
        }
    }
	return false;
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

	std::string endBoundary = _boundary;
	endBoundary.erase(endBoundary.find_last_not_of(" \n\r\t") + 1);
	endBoundary = endBoundary.substr(0, endBoundary.size()) + "--\r";
	// std::cout << BOLD_RED << "end boundary ::" << endBoundary << RESET <<  std::endl;
	size_t BodyheaderEndPos = body.find("\r\n\r\n");
	request.body = body.substr(BodyheaderEndPos + 4);
	// std::cout << BOLD_YELLOW << "this is the body " << request.body << RESET << std::endl;
	if (this->request.body.find(endBoundary) != std::string::npos)
	{
		std::size_t pos = request.body.rfind(endBoundary);
        	if (pos != std::string::npos)
        	    request.body.resize(pos);
		if (!request.body.empty() && request.body[request.body.size() - 1] == '\n')
                request.body.erase(request.body.size() - 2);
		//std::cout << "end of body  dont read anymore " << std::endl;
		return true;
	}
	return false; // this mean we still expecting more chunks
}

bool Client::HandlChunk()
{
	if (_bytesRead == 0 && !_boundary.empty()) //this is if the begining of boundry_body is in another chunk
	{
		// std::cout << "it came to if one " << std::endl;
		_bytesRead += this->_buffer.size(); 
		if (parse_body(this->_buffer))
			return true;
	}
	else if (_bytesRead != 0 &&!_boundary.empty()) // this if middle of boundry_body is in another chunk
	{
		// std::cout << "it came to if two " << std::endl;
		std::istringstream stream(this->_buffer);
		std::string line;
		_bytesRead += this->_buffer.size();
		std::string endBoundary = _boundary;
		endBoundary.erase(endBoundary.find_last_not_of(" \n\r\t") + 1);
		endBoundary = endBoundary.substr(0, endBoundary.size()) + "--\r";
		request.body += this->_buffer;
		// std::cerr << BOLD_RED << "end boundary :::::::" << endBoundary << RESET <<  std::endl;
		if (this->request.body.find(endBoundary) != std::string::npos)
		{
			std::size_t pos = request.body.rfind(endBoundary);
        	if (pos != std::string::npos)
        	    request.body.resize(pos);
			if (!request.body.empty() && request.body[request.body.size() - 1] == '\n')
                request.body.erase(request.body.size() - 2);
			// std::cout << "end of body  dont read anymore " << std::endl;
			return true;
		}
	}
	else // this would be for chunks that has no boundry 
	{
		// std::cout << "it came to if three " << std::endl;
		request.body += this->_buffer;
		_bytesRead += this->_buffer.size(); 
		if (_bytesRead == _targetBytes)
			return true;
		else if (_bytesRead > _targetBytes)
		{
			std::cerr << "Error: Bad request 400" << std::endl;
			request.statusError = 400;
			return true;
		}
	}
	// std::cout << "i am going to return falssxssssse " << std::endl;
	return false;
}

bool Client::handlingBody(std::string &body)
{

	if (stringToSizeT(request.headers["Content-Length"]) > _MaxBodySize)
    {
        std::cerr << "Error: Content-Length value is missing coming from handling body" << std::endl;
        request.statusError = 413;
        return true;
    }

	std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
	if (it != request.headers.end() && it->second.find("multipart/form-data") != std::string::npos)
	{
		_boundary = "--" + it->second.substr(it->second.find("boundary=") + 9);
		// std::cout << "this is the boundary :" << _boundary << std::endl;
	}

	if (body.size() > 0)// this is if the body comes with the headers in one chunk
	{
		if (!_boundary.empty())//that would mean we would have to upload a file 
		{
			if (parse_body(body)) // this would parse the body if the header and the body were in one chunk
			{
				//std::cout << "i am done with the body and it was all in one chunk" << std::endl;
				return true;
			}
		}
		else
		{
			std::cout << "i  upload a nomral data" << std::endl;
			request.body = body;
		}
	}

	_bytesRead = body.size();
	std::cout << "this is the body size " << _bytesRead << std::endl;
	_targetBytes = stringToSizeT(request.headers["Content-Length"]);
	std::cout << "this is the target bytes " << _targetBytes << std::endl;
	if (_bytesRead < _targetBytes)
	{
		_isChunked = true;
		std::cout << "set the is chunk to true " << std::endl;
		return false;
	}
	else if (_bytesRead == _targetBytes) /// i dont think this is needed but i will keep it for now
		return true;
	
	std::cout << "i am going to return false " << std::endl;
	return false;
}

void Client::allRecieved()
{

	if (!_isChunked && !_headersIsChunked)
	{
		// std::cout << "should only be here once" << std::endl;
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
		std::cout << BOLD_GREEN << "set the is all recieved to true" << RESET << std::endl;
		if (request.body.size() > 0)
		{
			if (!isCGIPost(request.path))
			{
				saveBodyToFile();
				request.statusError = 201;
			}
		}
		return;
	}
	isAllRecieved = false;
	std::cout << BOLD_RED << "set the is all recieved to false" << RESET << std::endl;
	if (request.body.size() > 0)
	{
		if (!isCGIPost(request.path))
			saveBodyToFile();
	}
}

bool Client::isCGIPost(std::string path)
{
	size_t pos = path.find("cgi-bin");
	std::string extension = get_file_extension(path);
	if (pos != std::string::npos && (extension == "py" || extension == "php" || extension == "pl" || extension == "sh"))
		return true;
	return false;
}