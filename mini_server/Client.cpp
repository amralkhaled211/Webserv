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

ServerBlock &Client::findServerBlock() // uses the Host header field -> server_name:port -> Host: localhost:8081
{
	std::string host = request.headers["Host"];
	size_t colon_pos = host.find(':');
	std::string server_name = (colon_pos != std::string::npos) ? host.substr(0, colon_pos) : host;
	std::string port = (colon_pos != std::string::npos) ? host.substr(colon_pos + 1) : "";

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		ServerBlock &server = *it;
		if (findInVector(server.getListen(), stringToInt(port)) && findInVector(server.getServerName(), server_name)) // here port is prioritized over server_name
			return server;
	}
	// this would be fixed later
	// std::cout << "reques:::" << request.headers["Host"] << std::endl;
	std::cout << "\033[1;31m" <<  "returning the first server?, This is a BUG " << "\033[0m" << std::endl;
	throw std::exception();
}

LocationBlock Client::findLocationBlock(std::vector<LocationBlock> &locations)
{
	std::vector<std::string> possibleReqLoc = possibleRequestedLoc(request.path); // other name: possibleReqLoc
	LocationBlock	location;
	std::string		fullPath;

	
	for (int i = 0; i < possibleReqLoc.size(); ++i)
	{
		for (std::vector<LocationBlock>::iterator it = locations.begin(); it != locations.end(); ++it)
		{
			location = *it;
			if (location.getPrefix() == possibleReqLoc[i]) // need to make sure the prefix is also cleaned from excess slashes
				return location;
		}
	}
	std::cout << BOLD_RED << "COULD NOT FIND LOCATION BLOCK" << RESET << std::endl;
	throw std::exception(); // this is temporary, will create a error handling mechanism
}


size_t parseSize(const std::string& sizeStr)
{
    std::string numberPart = sizeStr.substr(0, sizeStr.size() - 1);
    char suffix = sizeStr[sizeStr.size() - 1];

    size_t multiplier = 1;
    if (std::isdigit(suffix))
        numberPart = sizeStr;
    else
    {
        if (suffix == 'k' || suffix == 'K') 
            multiplier = 1024;
        else if (suffix == 'm' || suffix == 'M') 
            multiplier = 1024 * 1024;
        else if (suffix == 'g' || suffix == 'G') 
            multiplier = 1024 * 1024 * 1024;
    }

    // Convert numeric part to size_t
    size_t numericValue = 0;
    numericValue = stringToSizeT(numberPart);

    return numericValue * multiplier;
}


int Client::findMaxBodySize()
{
	try 
	{
		ServerBlock current_server = findServerBlock();
		LocationBlock location = findLocationBlock(current_server.getLocationVec());
		std::string max = location.getClientMaxBodySize();
		std::cout << "max body size: " << max << std::endl;
		_MaxBodySize = parseSize(max);
	}
	catch (std::exception &e)
	{
		std::cerr << "Error: could not find location block" << std::endl;
		request.statusError = 404;
		return 1;
	}
	return 0;
}

bool Client::parseHeadersAndBody()
{
	if (_isChunked)
	{
		// std::cout << "i am inside the chunked " << std::endl;
		if (HandlChunk())
			return true;
	}
	else if (request.method == "POST")
	{
		size_t headerEndPos = this->_buffer.find("\r\n\r\n");
		std::cout << BOLD_GREEN << "this buffer"<< this->_buffer << RESET << std::endl;
		if (headerEndPos == std::string::npos) // this would be an indcation that the headers would be on chunks 
		{
			std::cout << "headerEndPos not found" << std::endl;
			if (parseHeaders(this->_buffer))	
				return true;
			if (headersValidate(this->_buffer, request.method) || _newLineChecked) // if this true that means we have the headers and now we ganna do the same thing for the body
			{
				if (findMaxBodySize())
					return true;
				// std::cout << "i am inside the header vaildatio :: " << std::endl;
				if (request.statusError == 0) // this mean we are expecting a body if we dont have on then its not valid and we send a message
				{
					std::cout << "i am ready for the body : " << std::endl;
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
				std::cout << "i am stuck hrer " << std::endl;
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
			if (findMaxBodySize())
				return true;
			if (handlingBody(body))
			{
				std::cout << "giving true means end of body" << std::endl;
				return true;
			}
		}
	}
	else /// this will be for the GET request
	{
		//std::cout << "coming to the get validation " << std::endl;
		if (parseHeaders(this->_buffer))
			return true;
		if (headersValidate(this->_buffer, request.method))
			return true; // this would mean the headers are not chunked 
		else 
			_headersIsChunked = true;
	}
	std::cout << "we throwing this ::: " << std::endl;	
	return false;
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
//////////////////////////////////////////////////////////////////////////

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
	endBoundary = endBoundary.substr(0, endBoundary.size() - 1) + "--\r";
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
		std::cout << "end of body  dont read anymore " << std::endl;
		return true;
	}
	return false; // this mean we still expecting more chunks
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
		else if (_bytesRead > _targetBytes)
		{
			std::cerr << "Error: Bad request 400" << std::endl;
			request.statusError = 400;
			return true;
		}
	}
	std::cout << "i am going to return false " << std::endl;
	return false;
}

bool Client::handlingBody(std::string &body)
{
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
				// std::cout << "i am done with the body and it was all in one chunk" << std::endl;
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
void Client::saveBodyToFile()
{
	std::string filePath;
	if (!request.fileName.empty())
	{
		filePath = "/home/amalkhal/Webserv/website/upload/" + request.fileName;
		std::cout << "this is the file path " << filePath << std::endl;
	}
	else 
		filePath = "../website/upload/data.txt";

	//check if file already exists
    // std::ifstream infile(filePath.c_str());
    // if (infile.good())
    // {
	// 	//we might want to handle this differently like an error page or something
    //     std::cout << "File already exists: " << filePath << std::endl;
	// }
	// infile.close();

    // std::ofstream outFile(filePath.c_str());
	std::ofstream outFile;
    outFile.open(filePath.c_str(), std::ios::app | std::ios::binary);
    if (outFile.is_open())
	{
        // std::cout << "this is the body " << data << std::endl;
        outFile.write(request.body.c_str(), request.body.size());
        outFile.close();
    }
	else
	{
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
    }
	request.body.clear();
}

// void writeFile(const std::string& data)
// {
// 	std::string filePath;
// 	filePath = "../website/upload/" + request.fileName;
//     std::ofstream outFile;
//     outFile.open(filePath.c_str(), std::ios::app | std::ios::binary);
//     if (outFile.is_open()) {
//         // std::cout << "this is the body " << data << std::endl;
//         outFile.write(data.c_str(), data.size());
//         outFile.close();
//     } else {
//         std::cerr << "Error opening file for writing: " << filePath << std::endl;
//     }
// }
void Client::allRecieved()
{

	if (!_isChunked && !_headersIsChunked)
	{
		std::cout << "should only be here once" << std::endl;
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
			std::cout << "i am reading file " << std::endl;
			saveBodyToFile();
			// writeFile(request.body);
		}
		return;
	}
	std::cout << "i am not done yet" << std::endl;
	isAllRecieved = false;
	if (request.body.size() > 0)
	{
		std::cout << "i Am reading chuncked file " << std::endl;
		saveBodyToFile();
		// writeFile(request.body);
	}
}