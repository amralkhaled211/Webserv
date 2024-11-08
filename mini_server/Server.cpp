#include "Server.hpp"

Server::Server(std::vector<ServerBlock>& _serverVec) : _servers(_serverVec)
{
    //printServerVec(_servers);
}

Server::~Server()
{
    std::cout << "Server shutting down" << std::endl;
	for (int i = 0; i < _serverSockets.size(); i++)
        close(_serverSockets[i]);
}

bool isValidIPAddress(const std::string& ipAddress) // we use forbidden c functions here!
{
    int num, dots = 0;
    char *ptr;
    char ipCopy[16];
    strncpy(ipCopy, ipAddress.c_str(), 16);
    ipCopy[15] = '\0';

    if (ipAddress.empty())
        return false;

    ptr = strtok(ipCopy, ".");
    if (ptr == NULL)
        return false;

    while (ptr)
    {
        if (!isdigit(*ptr))
            return false;
        num = atoi(ptr);
        if (num >= 0 && num <= 255)
        {
            ptr = strtok(NULL, ".");
            if (ptr != NULL)
                dots++;
        }
        else
            return false;
    }

    return dots == 3;
}


int Server::create_and_configure_socket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
        throw std::runtime_error("Socket creation failed");

    make_socket_non_blocking(serverSocket);

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ||
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

    return serverSocket;
}

void Server::bindNamesWithPorts(std::vector<std::string>& serverName, std::vector<int> serverPort)
{
    if (serverName.size() == 0)
        serverName.push_back("localhost");

    for (std::vector<std::string>::iterator it_name = serverName.begin(); it_name != serverName.end(); ++it_name)
    {
        std::string name = *it_name;
        for (std::vector<int>::iterator it_port = serverPort.begin(); it_port != serverPort.end(); ++it_port)
        {
            int port = *it_port;
            int serverSocket = create_and_configure_socket();
            sockaddr_in serverAddress;
	        serverAddress.sin_family = AF_INET;
	        serverAddress.sin_port = htons(port);
            if (!isValidIPAddress(name))
                serverAddress.sin_addr.s_addr = INADDR_ANY;
            else
                serverAddress.sin_addr.s_addr = inet_addr(name.c_str());

	        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	        	throw std::runtime_error("Binding failed");

	        if (listen(serverSocket, 5) < 0)
	        	throw std::runtime_error("Listening failed");
            _serverSockets.push_back(serverSocket);
        }
    }
}


void Server::createSocket()
{
    //std::cout << "size of serverBlock :" << _servers.size() << std::endl;
    for(int i = 0; i < _servers.size(); i++)
    {
        bindNamesWithPorts(_servers[i].getServerName(), _servers[i].getListen());
    }
    Epoll epoll(_serverSockets, _servers);
}


void Server::run()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try
    {
        createSocket();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}