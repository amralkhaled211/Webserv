#include "Client.hpp"

int Client::validateContentLength()
{
	std::map<std::string, std::string>::iterator it = request.headers.find("Content-Length");
	
	if ((it != request.headers.end() && it->second.empty()))
	{
		std::cerr << "Error: Content-Length value is missing" << std::endl;
		return 400;
	}

	//std::cout << "validateConLEnt: " << it->second.size() << std::endl;
	//std::cout << "value of lenght <" << it->second << ">" << std::endl;
	//std::cout << "ascii of lenght <" << (int)it->second[0] << ">" << std::endl;
	std::string value = deleteSpaces(it->second);
	if (it != request.headers.end() && value.find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "Error: Content-Length value is not a number" << std::endl;
		return 400;
	}
	_targetBytes = stringToSizeT(value);
	return 0;
}

int Client::validateHost()
{
	std::map<std::string, std::string>::iterator it = request.headers.find("Host");
	if ((it != request.headers.end() && it->second.empty()))
	{
		std::cerr << "Error: host value is missing" << std::endl;
		return 400;
	}
	if (it == request.headers.end())
	{
		std::cerr << "HOST is missing" << std::endl;
		return 400;
	}
	std::cout << "validateConLEnt: " << it->second.size() << std::endl;
	std::cout << "value of lenght <" << it->second << ">" << std::endl;
	std::cout << "ascii of lenght <" << (int)it->second[0] << ">" << std::endl;
	return 0;
}

int Client::validateTransferEncoding()
{
	std::map<std::string, std::string>::iterator it = request.headers.find("Transfer-Encoding");
	if (it != request.headers.end() && it->second.empty())
	{
		std::cerr << "Transfer-Encoding is missing" << std::endl;
		return 400;
	}
	std::string chunk = deleteSpaces(it->second);
	if (chunk != "chunked")
	{
		std::cerr << "HTTP/1.1 501 Not Implemented" << std::endl;
		return 400;
	}
	return 0;
}


bool Client::handleChunkedTransferEncoding(std::istringstream &headerStream)
{
    std::string line;
    while (std::getline(headerStream, line))
	{
        if (!_chunkLengthRecieved)
		{
            std::cout << "reading the number " << std::endl;
            std::cout << BOLD_RED << "NUMBER : " << line << RESET << std::endl;
            line.erase(line.size() - 1); // this is to remove the \r
            if (!isHexadecimal(line) || line.empty())
			{
                std::cerr << "Error: not a hexadecimal" << std::endl;
                request.statusError = 400;
                return true;
            }
            _chunkLengthValue = hexStringToInt(deleteSpaces(line)); // convert hex to int
            if (_chunkLengthValue == 0 && _bytesRead > 0)
			{
                std::cout << "end of chunk" << std::endl;
                request.statusError = 0;
                return true;
            }
            _chunkLengthRecieved = true;
        }
		else
		{
            std::string chunk = line.erase(line.size() - 1);
            if (chunk.size() != _chunkLengthValue)
			{
                std::cout << "chunk length is not right, fix it" << std::endl;
                request.statusError = 400;
                return true;
            }
            request.body += chunk;
            _bytesRead += chunk.size();
            _chunkLengthRecieved = false;
        }
    }
    return false; // still expecting more chunks
}

bool Client::handleContentLength(std::istringstream &headerStream)
{
    std::string line;
    while (std::getline(headerStream, line))
	{
		if (line != "\r")
		{
        	request.body += line.erase(line.size() - 1); // remove the \r
			_bytesRead += request.body.size();
			if (_targetBytes != _bytesRead)
			{
                std::cout << _targetBytes << " : " << _bytesRead << std::endl;
                std::cerr << "Error: content length is too short" << std::endl;
                request.statusError = 400;
            }
			else
                request.statusError = 0;
            std::cout << "end of body" << std::endl;
            return true;
		}
    }
    return false; // still expecting more data
}

bool Client::bodyValidate(std::string &Buffer)
{
    if (request.headers.find("Content-Length") != request.headers.end())
    {
        if (stringToSizeT(request.headers["Content-Length"]) > _MaxBodySize)
        {
           //std::cerr << "Error: Content-Length value is missing" << std::endl;
            request.statusError = 413;
            return true;
        }
    }

    std::istringstream headerStream(Buffer);
    std::string line;
    if (request.body.empty() && !_newLineChecked)
    {
        std::getline(headerStream, line);
        std::cout << "skip the new line :" << std::endl;
        if (line == "\r") {
            _newLineChecked = true;
            return false;
        }
    }

    if (request.headers.find("Transfer-Encoding") != request.headers.end())
    {
        std::cout << "coming to the chunk encoding " << std::endl;
        if (!handleChunkedTransferEncoding(headerStream))
        {
            std::cout << "_bytes read ::" <<  _bytesRead << std::endl;
            if (_bytesRead > _MaxBodySize)
            {
                std::cerr << "Error: paload too large for the chunks" << std::endl;
                request.statusError = 413;
                return true;
            }
            return false;
        }
        else
            return true;
    }

    if (request.headers.find("Content-Length") != request.headers.end())
    {
        return handleContentLength(headerStream);
    }

    return false; // still expecting more data
}


bool Client::validateGetHeaders(std::string &buffer)
{
    std::istringstream headerStream(this->_buffer);
    std::string line;
    while (std::getline(headerStream, line))
    {
        if (line == "\r" && request.headers.find("Host") != request.headers.end() && !request.headers["Host"].empty()) // Check for the end of headers
        {
            // std::cout << "end of headers" << std::endl;
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
    return false;
}

bool Client::validatePostHeaders(std::string &buffer)
{
    std::istringstream headerStream(this->_buffer);
    std::string line;
    while (std::getline(headerStream, line))
    {
        if (line == "\r" && validateHost() == 0) // Check for the end of headers
        {

			// std::cout << "end of headers" << std::endl;
            if (request.headers.find("Content-Length") != request.headers.end() &&
                request.headers.find("Transfer-Encoding") == request.headers.end())
            {
                //std::cout << "doing the Content-length " << std::endl;
                request.statusError = validateContentLength();
                return true;
            }
            else if (request.headers.find("Content-Length") == request.headers.end() &&
                request.headers.find("Transfer-Encoding") != request.headers.end())
            {
                std::cout << "doing the transfer coding " << std::endl;
                request.statusError = validateTransferEncoding();
                return true;
            }
            else // this would mean it is conflicting or u didnt send the content length or transfer encoding
            {
                std::cout << "Content-Length and Transfer-Encoding conflicting " << std::endl;
                request.statusError = 400;
                return true;
            }
        }
        else if (line == "\r" && validateHost() == 400)// i need status code 411 if the content length is not there
        {
            std::cerr << "Error: minimum headers missing " << std::endl; 
            request.statusError = 400;
            return true;
        }
    }
    return false;
}

bool Client::headersValidate(std::string &buffer, std::string method)
{
    if (method == "GET" || method == "DELETE")
    {
        return validateGetHeaders(buffer);
    }
    else if (method == "POST")
    {
        return validatePostHeaders(buffer);
    }
   /*  else if (method == "DELETE")
    {
        return validateGetHeaders(buffer);
    } */
    return false;
}


bool isHexadecimal(const std::string& str)
{
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
        if (!std::isxdigit(*it)) {
            return false;
        }
    }
    return true;
}

int hexStringToInt(const std::string& hexStr)
{
    int intValue = 0;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> intValue;
    return intValue;
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


long parseSize(const std::string& sizeStr)
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
		_MaxBodySize = parseSize(max);
		// std::cout << "this is the max body size " << _MaxBodySize << std::endl;

	}
	catch (std::exception &e)
	{
		std::cerr << "Error: could not find location block" << std::endl;
		request.statusError = 404;
		return 1;
	}
	return 0;
}