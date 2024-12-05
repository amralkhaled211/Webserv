#include "Client.hpp"


int Client::validateContentLength()
{
	std::map<std::string, std::string>::iterator it = request.headers.find("Content-Length");
	if (it != request.headers.end() && it->second.empty())
	{
		std::cerr << "Error: Content-Length value is missing" << std::endl;
		return 400;
	}
	std::string value = deleteSpaces(it->second);
	value.erase(value.find_last_not_of(" \n\r\t") + 1);
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
	if (it != request.headers.end() && it->second.empty())
	{
		std::cerr << "HOST is missing" << std::endl;
		return 400;
	}
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
	std::cout << chunk  << std::endl;
	chunk.erase(chunk.size() - 1);
	std::cout << "chunk size :" << chunk.size() << std::endl;
	if (chunk != "chunked")
	{
		std::cerr << "HTTP/1.1 501 Not Implemented" << std::endl;
		return 400;
	}
	return 0;
}


bool Client::bodyValidate(std::string &Buffer)  // here i would change the logic a bit later && like checking for boundary and file name
{
	std::istringstream headerStream(Buffer);
	std::string line;
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

	// i should implement the trnasfer encoding 
	if (request.headers.find("Transfer-Encoding") == request.headers.end())
	{
		while (std::getline(headerStream, line))
		{
			if (!_chunkLengthRecieved)
			{
				std::getline(headerStream, line);
				_chunkLengthValue = stringToInt(line);/// i will change this to hex
				if (_chunkLengthValue == 0)
				{
					std::cout << "end of chunk" << std::endl;
					request.statusError = 0;
					return true;
				}
				_chunkLengthRecieved = true;
			}
			if (_chunkLengthRecieved)
			{
				std::getline(headerStream, line);
				std::string chunk = line.erase(line.size() - 1);
				if (chunk.size() > _chunkLengthRecieved || chunk.size() < _chunkLengthRecieved)
				{
					std::cout << "chunk lenght is not right fix it " << std::endl;
					request.statusError = 400;
					return true;
				}
				request.body += chunk;
				_chunkLengthRecieved = false;
			}
		}
	}
	// for content length
	if (request.headers.find("Content-Length") != request.headers.end())
	{
		request.body += Buffer;
		_bytesRead += request.body.size();
		while (std::getline(headerStream, line))
    	{
    	    if (line == "\r" && !request.body.empty()) // Check for the end of headers
			{
				std::cout << "end of body" << std::endl;
				request.body.erase(request.body.size() - 2); // this is to remove the last \r\n
				_bytesRead -= 2;
				if (_targetBytes != _bytesRead)
				{
					std::cout << _targetBytes << " : " << _bytesRead << std::endl;
					std::cerr << "Error: content length is too short " << std::endl;
					request.statusError = 400;
				}
				else
					request.statusError = 0;
				return true;
			}
    	}
	}
	return false; // this mean we still expecting more chunks
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
    	    if (line == "\r" && validateHost() == 0) // Check for the end of headers
			{
				if (request.headers.find("Content-Length") != request.headers.end() &&
					request.headers.find("Transfer-Encoding") == request.headers.end())
				{
						std::cout << "doing the Content-length " << std::endl;
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
				else // this would mean it is confilecting 
				{
					std::cout << "Content-Length and Transfer-Encoding confilicting "<< std::endl;
					request.statusError = 400;
					return true;
				}
			}
			else if (line == "\r" && (request.headers.find("Host") == request.headers.end() ||
						request.headers.find("Content-Length") == request.headers.end() || // i need status code 411 if the content length is not there
						request.headers.find("Content-Type") == request.headers.end()))// i need status code 411 if the content length is not there
			{
				std::cerr << "Error: minmum headers missing " << std::endl; 
				request.statusError = 400;
				return true;
			}
    	}
	}

	return false;
}








// if (method == "POST")
// 	{
// 		std::istringstream headerStream(this->_buffer);
// 		std::string line;
// 		while (std::getline(headerStream, line))
//     	{
//     	    if (line == "\r" && request.headers.find("Host") != request.headers.end() && !request.headers["Host"].empty() &&
// 					request.headers.find("Content-Length") != request.headers.end() && !request.headers["Content-Length"].empty() &&
// 					request.headers.find("Content-Type") != request.headers.end() && !request.headers["Content-Type"].empty()) // Check for the end of headers
// 			{
// 				std::cout << "end of headers" << std::endl;
// 				// request.statusError = validateContentType();
// 				if (validateContentLength() == 0)
// 					request.statusError = 0;
// 				else
// 					request.statusError = 400;
// 				return true;
// 			}
// 			else if (line == "\r" && (request.headers.find("Host") == request.headers.end() ||
// 						request.headers.find("Content-Length") == request.headers.end() || // i need status code 411 if the content length is not there
// 						request.headers.find("Content-Type") == request.headers.end()))
// 			{
// 				std::cerr << "Error: Bad request 400" << std::endl; 
// 				request.statusError = 400;
// 				return true;
// 			}
//     	}
// 	}

















//// i dont know if i wanna handle this or no 

// i might need this lateer
// int Client::validateContentType()
// {
// 	std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
// 	if (it != request.headers.end() && it->second.find("multipart/form-data") != std::string::npos) // this would mean there is a boundary
// 	{	
// 		size_t boundaryPos = it->second.find("boundary=");
//     	if (boundaryPos != std::string::npos)
//     	{
// 			if (it->second.substr(boundaryPos + 10).empty()) // here we did 10 cuz the we dont wanna count the new line
// 			{
// 				std::cerr << "Error: boundary value is missing in Content-Type header" << std::endl;
// 				return 400;
// 			}
//     	    _boundary = "--" + it->second.substr(boundaryPos + 9); 
// 			std::cout << "boundary is : " << _boundary << std::endl;
//     	}
//     	else
//     	{
//     	    std::cerr << "Error: boundary key is missing in Content-Type header" << std::endl;
//     	    return 400;
//     	}
// 	}
// 	// std::cout << "false return not expected" << std::endl;
// 	return 0;
// }

// void Client::validateBounderyBody()
// {
// 	std::string endBoundary = _boundary;
// 	endBoundary = endBoundary.substr(0, endBoundary.size() - 1) + "--\r"; // i have to tirm the /r first
// 	/// i wanna know valdate the first three lines in my body
// 	std::istringstream headerStream(request.body);
// 	std::string line;
// 	std::getline(headerStream, line);
// 	if (line != _boundary)
// 	{
// 		std::cerr << "bad boundery " << std::endl;
// 		request.statusError = 400;
// 		return;
// 	}
// 	// std::getline(headerStream, line);
// 	// if ()
// 	if (this->request.body.find(endBoundary) != std::string::npos)
// 	{
// 		std::size_t pos = request.body.rfind(endBoundary);
//     	if (pos != std::string::npos)
//     	    request.body.resize(pos);
// 		if (!request.body.empty() && request.body[request.body.size() - 1] == '\n')
//             request.body.erase(request.body.size() - 2);
// 		std::cout << "end of body  dont read anymore " << std::endl;
// 		request.statusError = 0;
// 	}
// 	else
// 	{
// 		std::cerr << "Error: boundery  problem " << std::endl;
// 		request.statusError = 400;
// 	}
// }