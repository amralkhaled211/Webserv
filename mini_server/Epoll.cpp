#include "Epoll.hpp"

void Epoll::init_epoll(const std::vector<int> &serverSockets)
{
    _epollFD = epoll_create1(0); // why are we using epoll_create1(0) --> with 0, it is the equivalent of using epoll_create()
    if (_epollFD == -1)
        throw std::runtime_error("epoll_create1");

    for (std::vector<int>::const_iterator it = serverSockets.begin(); it != serverSockets.end(); ++it) // registering the 
    {
        int sock = *it;
        struct epoll_event event;
        event.data.fd = sock;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, sock, &event) == -1)
        {
            std::cout << "here it failed " << std::endl;
            throw std::runtime_error("epoll_ctl");
        }
    }
}
void Epoll::acceptConnection(const std::vector<int> &serverSockets)
{
    init_epoll(serverSockets);
    while (serverRunning)
    {
        handleEpollEvents(serverSockets);
    }
}


void Epoll::killClient(int clientSocket)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->getClientFD() == clientSocket)
		{
			_clients.erase(it);
			break;
		}
	}
}

void Epoll::handleEpollEvents(const std::vector<int> &serverSockets)
{
   // std::cout << "Waiting for events" << std::endl;
    std::vector<struct epoll_event> _events(MAX_EVENTS);
    int n = epoll_wait(_epollFD, _events.data(), MAX_EVENTS, -1); // waits for I/O events, blocks the calling thread if no events are currently available
    if (n == -1)
    {
        if (errno == EINTR)
            return;
        throw std::runtime_error("epoll_wait");
    }
    for (int i = 0; i < n; ++i)
    {
        if (_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
        {
            std::cerr << "Error on fd " << _events[i].data.fd << ": ";
            if (_events[i].events & EPOLLERR) std::cerr << YELLOW_COLOR << "EPOLLERR " << RESET_COLOR;
            if (_events[i].events & EPOLLHUP) std::cerr << YELLOW_COLOR << "EPOLLHUP " << RESET_COLOR;
            if (_events[i].events & EPOLLRDHUP) std::cerr << YELLOW_COLOR << "EPOLLRDHUP "<< RESET_COLOR;
            std::cerr << std::endl;

            close(_events[i].data.fd);
            //std::cout << "Connection closed" << std::endl;
            //continue;
            return;
        }
        if (std::find(serverSockets.begin(), serverSockets.end(), _events[i].data.fd) != serverSockets.end())
            handleConnection(_events[i].data.fd);
        else if (_events[i].events & EPOLLIN )
            handleData(_events[i].data.fd);
        else if (_events[i].events & EPOLLOUT)
        {
            send(_events[i].data.fd, _buffer.c_str(), _buffer.size(), 0);
            killClient(_events[i].data.fd);
            close(_events[i].data.fd);
        }
    }
}
void Epoll::handleData(int client_fd)
{
    // std::cout << "Data received" << std::endl;
    requestHandle.receiveData(client_fd, _clients);
    Client client = requestHandle.findAllRecieved(_clients);
    if (client.getClientFD() != -1)
    {
        parser request = client.getRequest();
        _buffer = sendData.sendResponse(client.getClientFD(), _servers, client.getRequest(), _epollFD);
    }
}


void Epoll::handleConnection(int server_fd)
{
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
        make_socket_non_blocking(client_fd);
        struct epoll_event client_event;
        client_event.data.fd = client_fd;
        client_event.events = EPOLLIN ;
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
        {
            close(client_fd);
            
            throw std::runtime_error("epoll_ctl");
        }
        Client newClient;
        newClient.setClientFD(client_fd);
        _clients.push_back(newClient);
    }
}


Epoll::Epoll(const std::vector<int> &serverSockets, std::vector<ServerBlock> &servers) : _servers(servers)
{
    _epollFD = -1;
    acceptConnection(serverSockets);
}

Epoll::~Epoll()
{
    if (_epollFD != -1)
        close(_epollFD);
}

int make_socket_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, flags) == -1)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}