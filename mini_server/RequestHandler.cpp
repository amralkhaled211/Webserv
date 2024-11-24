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
	char Buffer[60000] = {0};
	int bytesReceived = recv(clientSocket, Buffer, sizeof(Buffer), 0);
	if (bytesReceived < 0)
		throw std::runtime_error("Receiving failed");
	this->_buffer.assign(Buffer, bytesReceived); // this buffer should go to the right client
	this->findClient(clientSocket, clients);
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