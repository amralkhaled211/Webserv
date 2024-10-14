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

void Server::bindNamesWithPorts(std::vector<std::string>& serverName, std::vector<int> serverPort, int serverSocket)
{
    for (std::vector<std::string>::iterator it_name = serverName.begin(); it_name != serverName.end(); ++it_name)
    {
        std::string name = *it_name;
        for (std::vector<int>::iterator it_port = serverPort.begin(); it_port != serverPort.end(); ++it_port)
        {
            sockaddr_in serverAddress;
	        serverAddress.sin_family = AF_INET;
	        serverAddress.sin_port = htons(*it_port);
	        serverAddress.sin_addr.s_addr = INADDR_ANY;
	        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	        {
	        	throw std::runtime_error("Binding failed");
	        }
	        if (listen(serverSocket, 5) < 0)
	        {
	        	throw std::runtime_error("Listening failed");
	        }
            _serverSockets.push_back(serverSocket);
        }
    }
}


void Server::createSocket()
{
    std::cout << "size of serverBlock :" << _servers.size() << std::endl;
    for(int i = 0; i < _servers.size(); i++)
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0)
	    	throw std::runtime_error("Socket creation failed");
	    make_socket_non_blocking(serverSocket);

        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0 ||
                setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        {
            close(serverSocket);
            throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
        }

        bindNamesWithPorts(_servers[i].getServerName(), _servers[i].getListen(), serverSocket);
    }
}
void Server::accept()
{
    Epoll epoll(_serverSockets, _servers);
}


void Server::run()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    try
    {
        createSocket();
        accept();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}






























// void Server::acceptConnection()
// {
//     epoll_fd = initialize_epoll(_serverSockets);
//     std::vector<struct epoll_event> events(MAX_EVENTS);

//     while (serverRunning)
//     {
//         int n = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
//         if (n == -1)
//         {
//             if (errno == EINTR)
//             {
//                 continue;
//             }
//             throw std::runtime_error("epoll_wait");
//         }
//         for (int i = 0; i < n; ++i)
//         {
//             if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
//             {
//                 close(events[i].data.fd);
//                 std::cout << "Connection closed" << std::endl;
//                 continue;
//             }
//             if (std::find(_serverSockets.begin(), _serverSockets.end(), events[i].data.fd) != _serverSockets.end())
//             {
//                 while (true)
//                 {
//                     int client_fd = accept(events[i].data.fd, NULL, NULL);
//                     if (client_fd == -1)
//                     {
//                         if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
//                         {
//                             break;
//                         }
//                         else
//                         {
//                             perror("accept");
//                             break;
//                         }
//                     }
//                     make_socket_non_blocking(client_fd);

//                     struct epoll_event client_event;
//                     client_event.data.fd = client_fd;
//                     client_event.events = EPOLLIN | EPOLLET;
//                     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
//                     {
//                         close(client_fd);
//                         throw std::runtime_error("epoll_ctl");
//                     }
//                     std::cout << "Connection accepted" << std::endl;
//                 }
//             }
//             else
//             {
//                 std::cout << "Data received" << std::endl;
//                 clientSocket = events[i].data.fd;
//                 requestHandle.receiveData(clientSocket);
//                 requestHandle.parseRequest();
//                 requestHandle.sendResponse(clientSocket);
//             }
//         }
//     }
// }













































// // void Server::acceptConnection()
// // {
// //     epoll_fd = epoll_create1(0);
// //     if (epoll_fd == -1)
// //     {
// //         throw std::runtime_error("epoll_create1");
// //     }

// //     for (std::vector<int>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
// //     {
// //         int sock = *it;
// //         struct epoll_event event;
// //         event.data.fd = sock;
// //         event.events = EPOLLIN | EPOLLET;

// //         if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1)
// //         {
// //             throw std::runtime_error("epoll_ctl");
// //         }
// //     }


// //     std::vector<struct epoll_event> events(MAX_EVENTS);

// //     while (serverRunning)
// //     {
// //         int n = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
// //         if (n == -1)
// //         {
// //             if (errno == EINTR)
// //             {
// //                 continue;
// //             }
// //             close(epoll_fd);
// //             throw std::runtime_error("epoll_wait");
// //         }
// //         for (int i = 0; i < n; ++i)
// //         {
// //             if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
// //             {
// //                 close(events[i].data.fd);
// //                 std::cout << "Connection closed" << std::endl;
// //                 continue;
// //             }
// //             if (std::find(_serverSockets.begin(), _serverSockets.end(), events[i].data.fd) != _serverSockets.end())
// //             {
// //                 while (true)
// //                 {
// //                     int client_fd = accept(events[i].data.fd, NULL, NULL);
// //                     if (client_fd == -1)
// //                     {
// //                         if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
// //                         {
// //                             break;
// //                         }
// //                         else
// //                         {
// //                             perror("accept");
// //                             break;
// //                         }
// //                         // perror("accept");
// //                         //     break;
// //                     }
// //                     make_socket_non_blocking(client_fd);

// //                     struct epoll_event client_event;
// //                     client_event.data.fd = client_fd;
// //                     client_event.events = EPOLLIN | EPOLLET;
// //                     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
// //                     {
// //                         close(client_fd);
// //                         throw std::runtime_error("epoll_ctl");
// //                     }
// //                     std::cout << "Connection accepted" << std::endl;
// //                 }
// //             }
// //             else
// //             {
// //                 std::cout << "Data received" << std::endl;

// //                 int server_fd = -1;
// //                 int server_index = -1;

// //                 // Find the corresponding server socket using std::find_if
// //                 for (std::vector<int>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
// //                 {
// //                     std::cout << "socket : " << *it << std::endl;
// //                     std::vector<epoll_event>::iterator event_it = std::find_if(events.begin(), events.end(), EventComparator(*it));
// //                     if (event_it != events.end())
// //                     {
// //                         server_fd = *it;
// //                         server_index = std::distance(_serverSockets.begin(), it);
// //                         std::cout << "server_index :" << server_index << std::endl;
// //                         break;
// //                     }
// //                     else
// //                     {
// //                         std::cout << "came here " << std::endl;
// //                     }
// //                 }

// //                 if (server_index != -1 && server_index < _servers.size())
// //                 {
// //                     ServerBlock &config = _servers[server_index];

// //                     clientSocket = events[i].data.fd;

// //                     requestHandle.receiveData(clientSocket, config);
// //                     requestHandle.parseRequest(); 
// //                     requestHandle.sendResponse(clientSocket);
// //                 }
// //                 else
// //                 {
// //                     std::cerr << "Error: Unable to find a valid server index." << std::endl;
// //                 }
// //             }
// //         }
// //     }
// // }


// void Server::acceptConnection()
// {
//     epoll_fd = epoll_create1(0);
//     if (epoll_fd == -1)
//     {
//         throw std::runtime_error("epoll_create1");
//     }

//     // Register server sockets to epoll
//     for (std::vector<int>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
//     {
//         int sock = *it;
//         struct epoll_event event;
//         event.data.fd = sock;
//         std::cout << "serverFDs: " << sock << std::endl;
//         event.events = EPOLLIN | EPOLLET;

//         if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1)
//         {
//             throw std::runtime_error("epoll_ctl");
//         }
//     }

//     std::vector<struct epoll_event> events(MAX_EVENTS);

//     while (serverRunning)
//     {
//         int n = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
//         if (n == -1)
//         {
//             if (errno == EINTR) continue;
//             close(epoll_fd);
//             throw std::runtime_error("epoll_wait");
//         }
//        // std::cout << "Number of events (fds) returned: " << n << std::endl;

//         for (int i = 0; i < n; ++i)
//         {
//             std::cout << "fd: " << events[i].data.fd << " i: " << i << std::endl;
//             if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
//             {
//                 close(events[i].data.fd);
//                 std::cout << "Connection closed" << std::endl;
//                 continue;
//             }

//             // Check if the event is from a server socket
//             std::vector<int>::iterator server_it = std::find(_serverSockets.begin(), _serverSockets.end(), events[i].data.fd);
//             if (server_it != _serverSockets.end())
//             {
//                 // Accept new client connections
//                 while (true)
//                 {
//                     int client_fd = accept(events[i].data.fd, NULL, NULL);
//                     std::cout << "clientFD: " << client_fd << std::endl;
//                     if (client_fd == -1)
//                     {
//                         if (errno == EAGAIN || errno == EWOULDBLOCK)
//                             break; // No more clients to accept

//                         perror("accept");
//                         break;
//                     }

//                     make_socket_non_blocking(client_fd);

//                     struct epoll_event client_event;
//                     client_event.data.fd = client_fd;
//                     client_event.events = EPOLLIN | EPOLLET;
//                     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
//                     {
//                         throw std::runtime_error("epoll_ctl");
//                     }
//                 }
//             }
//             else // Handle data from a client socket
//             {
//                 std::cout << "Data received" << std::endl;

//                 int clientSocket = events[i].data.fd;
//                 int server_index = -1;

//                 // Find the server index corresponding to the client socket's file descriptor
//                 for (std::vector<int>::iterator it = _serverSockets.begin(); it != _serverSockets.end(); ++it)
//                 {
//                     if (events[i].data.fd == *it)
//                     {
//                         server_index = std::distance(_serverSockets.begin(), it);
//                         break;
//                     }
//                 }

//                 if (server_index != -1 && server_index < _servers.size())
//                 {
//                     ServerBlock &config = _servers[server_index];

//                     requestHandle.receiveData(clientSocket, config);
//                     requestHandle.parseRequest(); 
//                     requestHandle.sendResponse(clientSocket);
//                 }
//                 else
//                 {
//                     std::cerr << "Error: Unable to find a valid server index." << std::endl;
//                 }
//             }
//         }
//     }
// }