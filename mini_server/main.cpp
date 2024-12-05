#include "Server.hpp"

#define DFLT_CONF_FILE  "conf_files/ali.conf"

bool serverRunning = true;

void signalHandler(int /* signum */)
{
    serverRunning = false;
}

int main(int ac, char* av[] /*, char* env[] */)
{
    try
    {
        if (ac > 2 || Parser::_generalErrors((ac == 2) ? std::string(av[1]) : std::string(DFLT_CONF_FILE)) == INVALID)
            throw(std::runtime_error("Error: Argument Count Not Two"));

		std::string     filename((ac == 2) ? av[1] : DFLT_CONF_FILE);
        Parser          httpBlock(filename); // other name: config

        httpBlock._parser();

        Server	server(httpBlock.getServerVec());
        initializeMimeTypesMap();
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << BOLD_RED << e.what() << RESET << std::endl;
    }
    return 0;
}
