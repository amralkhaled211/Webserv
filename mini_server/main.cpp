#include "Server.hpp"

bool serverRunning = true;

void signalHandler(int /* signum */)
{
    serverRunning = false;
}

int main()
{
    Server server;
    initializeMimeTypesMap();
    server.run();
    return 0;
}