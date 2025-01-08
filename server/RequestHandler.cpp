#include "RequestHandler.hpp"

RequestHandler::RequestHandler()
{
}
// i need here to check for the right client to send the buffer to
void RequestHandler::findClient(int clientSocket, std::vector<Client> &Clients) // better name: setBufferForCorrectClient()
{
	for (std::vector<Client>::iterator it = Clients.begin(); it != Clients.end(); ++it)
	{
		if (it->getClientFD() == clientSocket)
		{
			it->setBuffer(this->_buffer);
			it->allRecieved();
			if (it->getIsChunked())
				it->status = R_CHUNKS;
			else
				it->status = RECIEVING;
			break;
		}
	}
}

void RequestHandler::receiveData(int clientSocket, std::vector<Client> &clients)
{
	char Buffer[READ_CHUNK_SIZE] = {0};
	int bytesReceived = recv(clientSocket, Buffer, sizeof(Buffer), 0);
	if (bytesReceived < 0)
		throw std::runtime_error("Receiving failed");
	this->_buffer.assign(Buffer, bytesReceived); // this buffer should go to the right client
	this->findClient(clientSocket, clients); // must rename
}

Client &RequestHandler::findAllRecieved(std::vector<Client> clients)
{
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getIsAllRecieved() == true)
		{
			return (*it);
		}
	}
	//std::cout << BOLD_RED << "didnt find the right client " << RESET << std::endl;
	std::vector<Client>::iterator it = --clients.end();
	it->setClientFD(-1);
	return (*it);
}

std::vector<std::string>	possibleRequestedLoc(std::string uri)
{
	std::vector<std::string>	possibleReqLoc;
	size_t						lastSlash;

	removeExcessSlashes(uri);

	do
	{
		possibleReqLoc.push_back(uri);
		lastSlash = uri.find_last_of('/');
		uri = uri.substr(0, lastSlash);
	} while (!uri.empty());

	if (possibleReqLoc[possibleReqLoc.size() - 1] != "/")
		possibleReqLoc.push_back("/");

	return possibleReqLoc;
}