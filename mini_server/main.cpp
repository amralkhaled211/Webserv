#include "server.hpp"

bool serverRunning = true;

void signalHandler(int /* signum */)
{
    serverRunning = false;
}

int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try
    {
        initializeMimeTypesMap();
        Server server;
        server.createSocket();
        server.bindSocket();
        server.listenSocket();
        server.acceptConnection();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}







// Server* serverInstance = NULL;

// void handleSignal(int signal)
// {
//     if (serverInstance)
// 	{
//         delete serverInstance;
//         serverInstance = NULL;
//     }
//     std::cout << "Server shut down due to signal: " << signal << std::endl;
//     exit(signal);
// }

// int main()
// {
// 	try {
// 		serverInstance = new Server();
// 		signal(SIGINT, handleSignal);
// 		signal(SIGTERM, handleSignal);
// 		initializeMimeTypesMap();
// 		serverInstance->createSocket();
// 		serverInstance->bindSocket();
// 		serverInstance->listenSocket();
// 		serverInstance->acceptConnection();
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cerr << e.what() << std::endl;
// 	}
// 	delete serverInstance;
// }

// this is without handling signals
// #include "server.hpp"

// int main()
// {
// 	try {
// 		initializeMimeTypesMap();
// 		Server server;
// 		server.createSocket();
// 		server.bindSocket();
// 		server.listenSocket();
// 		server.acceptConnection();
// 	}
// 	catch (std::exception &e)
// 	{
// 		std::cerr << e.what() << std::endl;
// 	}
// }