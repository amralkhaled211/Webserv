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

Client& findClient(int clientFD, std::vector<Client>& clients) // careful, there is also a method of RequestHandler called findClient()
{
    size_t i;
    for (i = 0; i < clients.size(); ++i)
    {
        if (clientFD == clients[i].getClientFD())
            return clients[i];
    }
    return clients[--i]; // returning last one, should never happen!!
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
    DEBUG_Y "NEW ITERATION  --> ATTENTION: NUMBER OF FD WITH ACTIVE EVENTS: " << n << RESET << std::endl;
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
        if (std::find(serverSockets.begin(), serverSockets.end(), _events[i].data.fd) != serverSockets.end()) {
			DEBUG_G "Found Event on Socket FD: " << _events[i].data.fd << RESET << std::endl;
            handleConnection(_events[i].data.fd);
		}
        else if (_events[i].events & EPOLLIN ) {
			DEBUG_G "Found Event on Client FD: " << _events[i].data.fd << RESET << std::endl;
            handleData(_events[i].data.fd); // recieve Data & prep response
		}
        else if (_events[i].events & EPOLLOUT)
        {
			DEBUG_G "Sending data to Client FD: " << _events[i].data.fd << RESET << std::endl;

            Client &client = findClient(_events[i].data.fd, _clients);

            std::string &remainingResBuffer = client.getResponseBuffer();
            std::string sendNow;
            if (remainingResBuffer.size() > SEND_CHUNK_SIZE)
            {
                sendNow = remainingResBuffer.substr(0, SEND_CHUNK_SIZE);
                remainingResBuffer = remainingResBuffer.substr(SEND_CHUNK_SIZE);
            }
            else
                sendNow = remainingResBuffer;

			send(_events[i].data.fd, sendNow.c_str(), sendNow.size(), 0);

            if (remainingResBuffer.size() > SEND_CHUNK_SIZE)
            {
                // set this fd again to EPOLLOUT, to make sure epoll_wait() triggers
				struct epoll_event client_event;
				client_event.data.fd = _events[i].data.fd;
				client_event.events = EPOLLOUT;
				if (epoll_ctl(_epollFD, EPOLL_CTL_MOD, _events[i].data.fd, &client_event) == -1)
				{
					close(_events[i].data.fd);
					std::cout << BOLD_GREEN << "clientSocket Change mod : " << _events[i].data.fd << RESET << std::endl;
					std::cout << "epoll_ctl failed" << std::endl;
					throw std::runtime_error("epoll_ctl");
				}
                continue; // didn't send whole response, delay killing and closing
            }
            killClient(_events[i].data.fd);
            close(_events[i].data.fd);
        }
    }
}

void Epoll::handleData(int client_fd)
{
    // std::cout << "Data received" << std::endl;
    requestHandle.receiveData(client_fd, _clients);
    Client client = requestHandle.findAllRecieved(_clients); // we need the original
    if (client.getClientFD() != -1) // we only go on here once we recieved the whole request
    {
        // parser request = client.getRequest();
        std::string responseBuffer = sendData.sendResponse(client.getClientFD(), _servers, client.getRequest(), _epollFD); // return of this is the response
        findClient(client.getClientFD(), _clients).setResponseBuffer(responseBuffer);
    }
}


void Epoll::handleConnection(int server_fd) // we add additionally to the server_socket_fds also the cliend fd to the interest list (set of fds to watch)
{
    while (true) // why do we need to have accept in a loop?
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
        client_event.events = EPOLLIN; // by default lvl triggered, so the client fds are all level triggered
        if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
        {
            close(client_fd);
            
            throw std::runtime_error("epoll_ctl");
        }
        Client newClient;
        newClient.setClientFD(client_fd);	// creating new Client Object for the new client
        _clients.push_back(newClient);		// and adding it to the _clients vector
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