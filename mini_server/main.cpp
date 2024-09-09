#include "server.hpp"

int main()
{
	try {
		initializeMimeTypesMap();
		Server server;
		server.createSocket();
		server.bindSocket();
		server.listenSocket();
		server.acceptConnection();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}