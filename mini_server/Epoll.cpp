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
        event.events = EPOLLIN | EPOLLET; // this means this fd is gonna be for reading/recieving and edge triggered (must use non-blocking fds)
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, sock, &event) == -1) // about edge triggered: how do we manage to read everything from the fd buffer, when on the next interation, we won't be notified about he remainings in the fd buffer?
            throw std::runtime_error("epoll_ctl");
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
    std::cout << BOLD_YELLOW << "NEW ITERATION  --> ATTENTION: NUMBER OF FD WITH ACTIVE EVENTS: " << n << RESET << std::endl;
    for (int i = 0; i < n; ++i) {
        std::cout << _events[i].data.fd << ", ";
    }
    std::cout << std::endl;
    for (int i = 0; i < n; ++i)
    {
        if (_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) // what do these errors/flags mean?
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
        // std::cout << BOLD_YELLOW << "EVENT ON THIS FD: " << _events[i].data.fd << RESET << std::endl;
        if (std::find(serverSockets.begin(), serverSockets.end(), _events[i].data.fd) != serverSockets.end()) // find IF and WHICH serverFD the request came to, and accept it in 
        {
            std::cout << BOLD_GREEN << "COMMUNICATION AT THIS SOCKET: " << _events[i].data.fd << RESET << std::endl;
            handleConnection(_events[i].data.fd); // here we accept the connection, and get a client_fd, from which we read in the next if
        }
        // else if (_events[i].events & EPOLLIN || _events[i].events & EPOLLET)
        else if (_events[i].events & EPOLLIN ) // it has to be else if, because we want to read from the client_fd, not the serverSocketFD
        {
            std::cout << BOLD_RED << "RECIEVING REQUEST ON SOCKET FD " << _events[i].data.fd << RESET << std::endl;
            handleData(_events[i].data.fd); // handle incoming request
        }
        else if (_events[i].events & EPOLLOUT) // handle outgoing response
        {
            // std::cout << "Sending data" << std::endl;
            std::cout << BOLD_RED << "SENDING RESPONSE ON SOCKET FD " << _events[i].data.fd << RESET << std::endl;
            send(_events[i].data.fd, _buffer.c_str(), _buffer.size(), 0);
            close(_events[i].data.fd);
            //std::cout << "Connection closed" << std::endl;
        }
    }
}

void Epoll::handleData(int client_fd)
{
    // std::cout << "Data received" << std::endl;
    requestHandle.receiveData(client_fd);
    if (requestHandle.parseRequest())
    {
        //std::cout << "ready to send . " << std::endl;
        _buffer = sendData.sendResponse(client_fd, _servers, requestHandle.getRequest(), _epollFD); // consider change of name to prepareResponse()
    }
}


void Epoll::handleConnection(int server_fd) // we add also the cliend fd to the interest list / set of fds to watch
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
        std::cout << "CLIENT FD: " << client_fd << std::endl;
        make_socket_non_blocking(client_fd);
        struct epoll_event client_event;
        client_event.data.fd = client_fd;
        client_event.events = EPOLLIN ;
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
        {
            close(client_fd);
            throw std::runtime_error("epoll_ctl");
        }
        _clientFDs.push_back(client_fd);
        // std::cout << "Connection accepted" << std::endl;
    }
}


Epoll::Epoll(const std::vector<int> &serverSockets, std::vector<ServerBlock> &servers) : _servers(servers)
{
    _epollFD = -1;
    // printServerVec(_servers);
    acceptConnection(serverSockets);
}

Epoll::~Epoll()
{
    if (_epollFD != -1)
        close(_epollFD);
    for (std::vector<int>::iterator it = _clientFDs.begin(); it != _clientFDs.end(); ++it)
    {
        epoll_ctl(_epollFD, EPOLL_CTL_DEL, *it, NULL);
        close(*it);
    }
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