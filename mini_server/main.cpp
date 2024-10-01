#include "Server.hpp"

bool serverRunning = true;

void signalHandler(int /* signum */)
{
    serverRunning = false;
}

int main(int ac, char* av[] /*, char* env[] */)
{
    try
    {
		std::string	filename(av[1]);
        if (ac != 2 && Parser::_generalErrors(filename) == INVALID)
            throw(std::runtime_error("Error: Argument Count Not Two"));

        Parser	httpBlock(filename); // other name: config
        httpBlock._parser();


        Server	server;
        initializeMimeTypesMap();
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}