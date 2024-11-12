#include "StatusMsg.hpp"

StatusMsg::StatusMsg() {

	// better names later
	_oneHunderds();
	_twoHunderds(); // successful
	_threeHunderds(); // redirection
	_fourHunderds(); // error occures
	_fiveHunderds();
}

StatusMsg::~StatusMsg() {

}


void	StatusMsg::_oneHunderds() {
	std::vector<std::string> msg;

	// 100 Continue
	msg.push_back("Continue");
	msg.push_back("The server has received the request headers, and the client should proceed to send the request body.");
	_statusMsg[100] = msg;
	msg.clear();

	// 101 Switching Protocols
	msg.push_back("Switching Protocols");
	msg.push_back("The requester has asked the server to switch protocols and the server has agreed to do so.");
	_statusMsg[101] = msg;
	msg.clear();
}

void	StatusMsg::_twoHunderds() {
	std::vector<std::string> msg;

	// 200 OK
	msg.push_back("OK");
	msg.push_back("");
	_statusMsg[200] = msg;
	msg.clear();

	// 201 Created
	msg.push_back("Created");
	msg.push_back("");
	_statusMsg[201] = msg;
	msg.clear();

}

void	StatusMsg::_threeHunderds() {
	std::vector<std::string> msg;

	// 300 Multiple Choices
	msg.push_back("Multiple Choices");
	msg.push_back("The request has more than one possible response. The user-agent or user should choose one of them.");
	_statusMsg[300] = msg;
	msg.clear();

	// 301 Moved Permanently
	msg.push_back("Moved Permanently");
	msg.push_back("The URL of the requested resource has been changed permanently. The new URL is given in the response.");
	_statusMsg[301] = msg;
	msg.clear();

	// 302 Found
	msg.push_back("Found");
	msg.push_back("The URL of the requested resource has been changed temporarily. Further changes in the URL might be made in the future.");
	_statusMsg[302] = msg;
	msg.clear();

	// 303 See Other
	msg.push_back("See Other");
	msg.push_back("The server sent this response to direct the client to get the requested resource at another URI with a GET request.");
	_statusMsg[303] = msg;
	msg.clear();

	// 304 Not Modified
	msg.push_back("Not Modified");
	msg.push_back("This is used for caching purposes. It tells the client that the response has not been modified, so the client can continue to use the same cached version of the response.");
	_statusMsg[304] = msg;
	msg.clear();

	// 307 Temporary Redirect
	msg.push_back("Temporary Redirect");
	msg.push_back("The server is currently responding to the request with a different URI. The client should use the same method for the request.");
	_statusMsg[307] = msg;
	msg.clear();

	// 308 Permanent Redirect
	msg.push_back("Permanent Redirect");
	msg.push_back("The server is currently responding to the request with a different URI. The client should use the same method for the request.");
	_statusMsg[308] = msg;
	msg.clear();
}

void	StatusMsg::_fourHunderds() {
	std::vector<std::string> msg;

	// 400 Bad Request
	msg.push_back("Bad Request");
	msg.push_back("The server could not understand the request due to invalid syntax.");
	_statusMsg[400] = msg;
	msg.clear();

	// 401 Unauthorized
	msg.push_back("Unauthorized");
	msg.push_back("The client must authenticate itself to get the requested response.");
	_statusMsg[401] = msg;
	msg.clear();

	// 403 Forbidden
	msg.push_back("Forbidden");
	msg.push_back("The client does not have access rights to the content.");
	_statusMsg[403] = msg;
	msg.clear();

	// 404 Not Found
	msg.push_back("Not Found");
	msg.push_back("The server can not find the requested resource.");
	_statusMsg[404] = msg;
	msg.clear();
}

void	StatusMsg::_fiveHunderds() {
	std::vector<std::string> msg;

	// 500 Internal Server Error
	msg.push_back("Internal Server Error");
	msg.push_back("The server has encountered a situation it doesn't know how to handle.");
	_statusMsg[500] = msg;
	msg.clear();

	// 502 Bad Gateway
	msg.push_back("Bad Gateway");
	msg.push_back("The server, while acting as a gateway or proxy, received an invalid response from the upstream server.");
	_statusMsg[502] = msg;
	msg.clear();

	// 503 Service Unavailable
	msg.push_back("Service Unavailable");
	msg.push_back("The server is not ready to handle the request.");
	_statusMsg[503] = msg;
	msg.clear();

	// 504 Gateway Timeout
	msg.push_back("Gateway Timeout");
	msg.push_back("The server, while acting as a gateway or proxy, did not get a response in time from the upstream server.");
	_statusMsg[504] = msg;
	msg.clear();
}
