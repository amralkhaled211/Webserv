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
        if (/* ac != 2 || */ Parser::_generalErrors(std::string(av[1])) == INVALID)
            throw(std::runtime_error("Error: Argument Count Not Two"));

		std::string     filename(av[1]);
        Parser          httpBlock(filename); // other name: config

        httpBlock._parser();

        // std::cout << "BACK IN MAIN\n";

        // Server	server;
        // initializeMimeTypesMap();
        // server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}