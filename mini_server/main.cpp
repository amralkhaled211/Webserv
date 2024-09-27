#include "Server.hpp"

bool serverRunning = true;

void signalHandler(int /* signum */)
{
    serverRunning = false;
}

int main(int ac, char* av[])
{
    try
    {
        Server server;

        if (ac != 2 && Parser::_generalErrors(std::string(av[1])) == INVALID)
            throw(std::runtime_error("Error: Argument Count Not Two"));
        initializeMimeTypesMap();
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}