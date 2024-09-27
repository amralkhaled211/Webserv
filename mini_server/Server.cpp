#include "Server.hpp"

Server::Server()
{
	serverSocket = 0;
    //serverSocket2 = 1;
	clientSocket = 0;
}

Server::~Server()
{
    std::cout << "Server shutting down" << std::endl;
	if (serverSocket != -1)
    {
        // std::cout << "Closing server socket" << std::endl;
        close(serverSocket);
    }

    if (clientSocket != -1)
    {
        // std::cout << "Closing client socket" << std::endl;
        close(clientSocket);
    }

	if (epoll_fd != -1)
	{
        // std::cout << "Closing epoll file descriptor" << std::endl;
		close(epoll_fd);
	}
    // if (serverSocket2 != -1)
    // {
    //     // std::cout << "Closing server socket" << std::endl;
    //     close(serverSocket2);
    // }
}

void Server::createSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		throw std::runtime_error("Socket creation failed");
	}
	make_socket_non_blocking(serverSocket);
}

void Server::bindSocket()
{
    //this function would prevent the "Address already in use" && "binding failed" error
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }


	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		throw std::runtime_error("Binding failed");
	}
}

void Server::listenSocket()
{
	if (listen(serverSocket, 5) < 0)
	{
		throw std::runtime_error("Listening failed");
	}
}

void Server::acceptConnection()
{
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        throw std::runtime_error("epoll_create1");
    }

    std::vector<int> serverSockets;
    serverSockets.push_back(serverSocket);
    //serverSockets.push_back(serverSocket2);

    for (std::vector<int>::iterator it = serverSockets.begin(); it != serverSockets.end(); ++it)
    {
        int sock = *it;
        struct epoll_event event;
        event.data.fd = sock;
        event.events = EPOLLIN | EPOLLET;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1)
        {
            close(epoll_fd);
            throw std::runtime_error("epoll_ctl");
        }
    }


    std::vector<struct epoll_event> events(MAX_EVENTS);

    while (serverRunning)
    {
        int n = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            close(epoll_fd);
            throw std::runtime_error("epoll_wait");
        }
        for (int i = 0; i < n; ++i)
        {
            if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
            {
                close(events[i].data.fd);
                std::cout << "Connection closed" << std::endl;
                continue;
            }
            if (std::find(serverSockets.begin(), serverSockets.end(), events[i].data.fd) != serverSockets.end())
            {
                while (true)
                {
                    int client_fd = accept(events[i].data.fd, NULL, NULL);
                    if (client_fd == -1)
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            break;
                        }
                        else
                        {
                            perror("accept");
                            break;
                        }
                    }
                    make_socket_non_blocking(client_fd);

                    struct epoll_event client_event;
                    client_event.data.fd = client_fd;
                    client_event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
                    {
                        close(client_fd);
                        throw std::runtime_error("epoll_ctl");
                    }
                    std::cout << "Connection accepted" << std::endl;
                }
            }
            else
            {
                std::cout << "Data received" << std::endl;
                clientSocket = events[i].data.fd;
                requestHandle.receiveData(clientSocket);
                requestHandle.parseRequest();
                requestHandle.sendResponse(clientSocket);
            }
        }
    }
}


int make_socket_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

void Server::run()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try
    {
        createSocket();
        bindSocket();
        listenSocket();
        //createSocket2();
        //bindSocket2();
        //listenSocket2();
        acceptConnection();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

// creating another socket for testing purposes

// void Server::createSocket2()
// {
// 	serverSocket2 = socket(AF_INET, SOCK_STREAM, 0);
// 	if (serverSocket2 < 0)
// 	{
// 		throw std::runtime_error("Socket creation failed"); 
// 	}
//     make_socket_non_blocking(serverSocket2);
// }

// void Server::bindSocket2()
// {
//     //this function would prevent the "Address already in use" && "binding failed" error
//     int opt = 1;
//     if (setsockopt(serverSocket2, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
//     {
//         throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
//     }


// 	sockaddr_in serverAddress;
// 	serverAddress.sin_family = AF_INET;
// 	serverAddress.sin_port = htons(8081);
// 	serverAddress.sin_addr.s_addr = INADDR_ANY;
// 	if (bind(serverSocket2, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
// 	{
// 		throw std::runtime_error("Binding failed");
// 	}
// }


// void Server::listenSocket2()
// {
// 	if (listen(serverSocket2, 5) < 0)
// 	{
// 		throw std::runtime_error("Listening failed");
// 	}
// }