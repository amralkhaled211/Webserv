#include "Epoll.hpp"

Epoll::Epoll(const std::vector<int>& serverSockets, std::vector<ServerBlock>& servers) : _servers(servers)
{
    _epollFD = -1;
    //printServerVec(_servers);
    acceptConnection(serverSockets);
}

Epoll::~Epoll()
{
    close(_epollFD);
}

void Epoll::init_epoll(const std::vector<int>& serverSockets)
{
	_epollFD = epoll_create1(0);
    if (_epollFD == -1)
       throw std::runtime_error("epoll_create1");

    for (std::vector<int>::const_iterator it = serverSockets.begin(); it != serverSockets.end(); ++it)
    {
        int sock = *it;
        struct epoll_event event;
        event.data.fd = sock;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, sock, &event) == -1)
            throw std::runtime_error("epoll_ctl");
    }
}

void Epoll::handleEpollEvents(const std::vector<int>& serverSockets)
{
	std::vector<struct epoll_event>  _events(MAX_EVENTS);

	int n = epoll_wait(_epollFD, _events.data(), MAX_EVENTS, -1);
    if (n == -1)
    {
        if (errno == EINTR)
            return;
        throw std::runtime_error("epoll_wait");
    }
    // std::cout << "Number of events (fds) returned: " << n << std::endl;
    for (int i = 0; i < n; ++i)
	{
        if (_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
		{
            close(_events[i].data.fd);
            std::cout << "Connection closed" << std::endl;
            continue;
        }
       // std::cout << "fd: " << _events[i].data.fd << " i: " << i << std::endl;
        if (std::find(serverSockets.begin(), serverSockets.end(), _events[i].data.fd) != serverSockets.end())
		{
           handleConnection(_events[i].data.fd);
        }
		else
		{
            std::cout << "_events[i].data.fd: " << _events[i].data.fd << std::endl;
           handleData(_events[i].data.fd);
        }
    }
}
void Epoll::handleConnection(int server_fd)
{
   std::cout << "server_fd: " << server_fd << std::endl;
	while (true)
	{
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                break;
			else
                perror("accept");
                break;
        }
        std::cout << "client_fd: " << client_fd << std::endl;
        make_socket_non_blocking(client_fd);
        struct epoll_event client_event;
        client_event.data.fd = client_fd;
        client_event.events = EPOLLIN;
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
		{
            close(client_fd);
            throw std::runtime_error("epoll_ctl");
        }
        std::cout << "Connection accepted" << std::endl;
        // Client client(client_fd, _servers[0]);
        // _clients.push_back(client);
    }
}

void Epoll::handleData(int client_fd)
{
    std::cout << "Data received" << std::endl;
    //std::cout << "client_fd in handle date " << client_fd << std::endl; 
    //std::cout << "_clients size :" << _clients.size() << std::endl;
    //std::cout << "value of cfd :" << _clients[0].getClientFD() << std::endl;
    requestHandle.receiveData(client_fd);
    requestHandle.parseRequest();
    requestHandle.sendResponse(client_fd, _servers);
}

void Epoll::acceptConnection(const std::vector<int>& serverSockets)
{
	init_epoll(serverSockets);
    while (serverRunning)
	{
        handleEpollEvents(serverSockets);
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