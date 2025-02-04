#include "Server.hpp"

Server::Server(std::vector<ServerBlock>& _serverVec) : _servers(_serverVec)
{
    //printServerVec(_servers);
    //printServerVec(_servers);
}

Server::~Server()
{
    std::cout << "Server shutting down" << std::endl;
	for (size_t i = 0; i < _serverSockets.size(); i++)
        close(_serverSockets[i]);
}


bool isValidIPAddress(const std::string& ipAddress) // we use c functions here!
{
    int num, dots = 0;
    char *ptr;
    char ipCopy[16];
    std::strncpy(ipCopy, ipAddress.c_str(), 16);
    ipCopy[15] = '\0';

    if (ipAddress.empty())
        return false;

    ptr = std::strtok(ipCopy, ".");
    if (ptr == NULL)
        return false;

    while (ptr)
    {
        if (!std::isdigit(*ptr))
            return false;
        num = std::atoi(ptr);
        if (num >= 0 && num <= 255)
        {
            ptr = std::strtok(NULL, ".");
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

    int opt = 1; // 1024 * 1024;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

    return serverSocket;
}

std::string resolveFromHostsFile(const std::string& hostname) {
    std::ifstream hostsFile("/etc/hosts");
    if (!hostsFile) {
        std::cerr << "Error opening /etc/hosts file." << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(hostsFile, line)) {

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string ip;
        std::string entry;

        if (!(iss >> ip)) {
            continue;
        }

        while (iss >> entry) {
            if (entry == hostname) {
                return ip;
            }
        }
    }

    return "";
}

void Server::bindNamesWithPorts(std::vector<std::string>& serverName, std::vector<int> serverPort, ServerBlock &currServer)
{
    if (serverName.size() == 0)
        serverName.push_back("localhost");

    for (std::vector<std::string>::iterator it_name = serverName.begin(); it_name != serverName.end(); ++it_name)
    {
        std::string name = *it_name;
        for (std::vector<int>::iterator it_port = serverPort.begin(); it_port != serverPort.end(); ++it_port)
        {
            int port = *it_port;
            sockaddr_in serverAddress; // configuration for the socket
	        serverAddress.sin_family = AF_INET;
	        serverAddress.sin_port = htons(port);

            if (!isValidIPAddress(name))
            {
                // struct addrinfo hints, *res;
                // memset(&hints, 0, sizeof(hints));
                // hints.ai_family = AF_INET;
                // hints.ai_socktype = SOCK_STREAM;

                // int status = getaddrinfo(name.c_str(), NULL, &hints, &res);
                std::string ip = resolveFromHostsFile(name);
                if (ip == "")
                {
                    serverAddress.sin_addr.s_addr = INADDR_ANY;
                }
                else
                {
                    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
                    // serverAddress.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
                    // freeaddrinfo(res);
                }
            }
            else
            {
                serverAddress.sin_addr.s_addr = inet_addr(name.c_str());
            }

            int serverSocket = create_and_configure_socket();

            int bindRC = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	        if (bindRC < 0)
            {
                close(serverSocket);
                std::cerr << "Binding failed: " << strerror(errno) << std::endl;
                std::cout << "First / Default virtual-server will be used for " << name << ":" << port << std::endl;
                continue;
	        	// throw std::runtime_error(std::string("Binding failed: ") + strerror(errno));
            }

	        if (listen(serverSocket, 5) < 0)
	        	throw std::runtime_error("Listening failed");
            
            currServer.setHostInMap(serverSocket, name + ":" + intToString(port));

            _serverSockets.push_back(serverSocket);

            std::cout << "visit site on http://" << name + ":" + intToString(port) << " (fd: " << serverSocket << ")"<< std::endl;
        }
    }
}


void Server::createSocket()
{

    //std::cout << "size of serverBlock :" << _servers.size() << std::endl;
    for(size_t i = 0; i < _servers.size(); i++)
    {
        bindNamesWithPorts(_servers[i].getServerName(), _servers[i].getListen(), _servers[i]);
    }

    if (_serverSockets.empty())
        throw std::runtime_error("No server sockets created");
    
    //std::cout << BOLD_GREEN << "ALL THE CREATED SOCKETS: ";
    //for (size_t i = 0; i < this->_serverSockets.size(); ++i) {
    //    std::cout << _serverSockets[i] << ", ";
    //}
    // std::cout << std::endl;
    // Epoll epoll(_serverSockets, _servers); // maybe we should start "everything" in run(), not in createSocket()
}



void Server::run()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try
    {
        createSocket();
        Epoll epoll(_serverSockets, _servers);
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}