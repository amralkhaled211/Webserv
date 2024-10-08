#include "Epoll.hpp"

Epoll::Epoll(const std::vector<int>& serverSockets) : serverSockets(serverSockets)
{
	acceptConnection();
}

Epoll::~Epoll()
{
	close(_epollFD);
}

void Epoll::init_epoll()
{
	_epollFD = epoll_create1(0);
    if (_epollFD == -1) {
        throw std::runtime_error("epoll_create1");
    }

    for (std::vector<int>::const_iterator it = serverSockets.begin(); it != serverSockets.end(); ++it)
    {
        int sock = *it;
        struct epoll_event event;
        event.data.fd = sock;
        event.events = EPOLLIN | EPOLLET;

        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, sock, &event) == -1)
        {
            throw std::runtime_error("epoll_ctl");
        }
    }
}

void Epoll::handleConnection(int server_fd)
{
	while (true)
	{
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
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
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
		{
            close(client_fd);
            throw std::runtime_error("epoll_ctl");
        }
        std::cout << "Connection accepted" << std::endl;
    }
}


void Epoll::handleData(int client_fd)
{
    std::cout << "Data received" << std::endl;
    requestHandle.receiveData(client_fd);
    requestHandle.parseRequest();
    requestHandle.sendResponse(client_fd);
}

void Epoll::handleEpollEvents( std::vector<struct epoll_event>& _events)
{
	int n = epoll_wait(_epollFD, _events.data(), MAX_EVENTS, -1);
    if (n == -1) {
        if (errno == EINTR) {
            return;
        }
        throw std::runtime_error("epoll_wait");
    }
    for (int i = 0; i < n; ++i)
	{
        if (_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
		{
            close(_events[i].data.fd);
            std::cout << "Connection closed" << std::endl;
            continue;
        }
        if (std::find(serverSockets.begin(), serverSockets.end(), _events[i].data.fd) != serverSockets.end())
		{
            handleConnection(_events[i].data.fd);
        }
		else
		{
            handleData(_events[i].data.fd);
        }
    }
}

void Epoll::acceptConnection()
{
	init_epoll();
	std::vector<struct epoll_event>  events(MAX_EVENTS);
    while (serverRunning)
	{
        handleEpollEvents(events);
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