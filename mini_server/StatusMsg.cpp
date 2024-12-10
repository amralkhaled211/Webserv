#include "StatusMsg.hpp"

StatusMsg::StatusMsg()
{

	// better names later
	_oneHunderds();
	_twoHunderds();	  // successful
	_threeHunderds(); // redirection
	_fourHunderds();  // error occures
	_fiveHunderds();
}

StatusMsg::~StatusMsg()
{
}

void StatusMsg::_oneHunderds()
{
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

void StatusMsg::_twoHunderds()
{
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

	// 203 Non-Authoritative Information
    msg.push_back("Non-Authoritative Information");
    msg.push_back("The request was successful but the enclosed payload has been modified from that of the origin server's 200 OK response by a transforming proxy.");
    _statusMsg[203] = msg;
    msg.clear();

    // 204 No Content
    msg.push_back("No Content");
    msg.push_back("The server successfully processed the request and is not returning any content.");
    _statusMsg[204] = msg;
    msg.clear();

    // 205 Reset Content
    msg.push_back("Reset Content");
    msg.push_back("The server successfully processed the request, but is not returning any content and requires that the requester reset the document view.");
    _statusMsg[205] = msg;
    msg.clear();

    // 206 Partial Content
    msg.push_back("Partial Content");
    msg.push_back("The server is delivering only part of the resource due to a range header sent by the client.");
    _statusMsg[206] = msg;
    msg.clear();

    // 207 Multi-Status (WebDAV)
    msg.push_back("Multi-Status");
    msg.push_back("The message body that follows is an XML message and can contain a number of separate response codes, depending on how many sub-requests were made.");
    _statusMsg[207] = msg;
    msg.clear();

    // 208 Already Reported (WebDAV)
    msg.push_back("Already Reported");
    msg.push_back("The members of a DAV binding have already been enumerated in a previous reply to this request, and are not being included again.");
    _statusMsg[208] = msg;
    msg.clear();

    // 226 IM Used (HTTP Delta encoding)
    msg.push_back("IM Used");
    msg.push_back("The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance.");
    _statusMsg[226] = msg;
    msg.clear();
}

void StatusMsg::_threeHunderds()
{
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

void StatusMsg::_fourHunderds()
{
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
	
	// 405 Method Not Allowed
	msg.push_back("Method Not Allowed");
	msg.push_back("The request method is known by the server but has been disabled and cannot be used.");
	_statusMsg[405] = msg;
	msg.clear();
}

void StatusMsg::_fiveHunderds()
{
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

	 msg.push_back("Infinite Loop Detected");
	 msg.push_back("The server has detected an infinite loop while processing the request.");
	 _statusMsg[508] = msg;
	 msg.clear();

}
