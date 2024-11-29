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
		_clFDs.push_back(sock);
	}
}
void Epoll::acceptConnection(const std::vector<int> &serverSockets)
{
	init_epoll(serverSockets);
	while (serverRunning)
	{
		// DEBUG_Y "B<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>\n";
		handleEpollEvents(serverSockets);
		// DEBUG_R "E<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>\n";
	}
}

void Epoll::killClient(int clientSocket)
{
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->getClientFD() == clientSocket)
		{
			_clients.erase(it);
			removeFD(clientSocket);
			break;
		}
	}
	for (std::vector<int>::iterator it = _clFDs.begin(); it != _clFDs.end(); ++it)
	{
		if (*it == clientSocket)
		{
			_clFDs.erase(it);
			break;
		}
	}
}

Client &findClient(int clientFD, std::vector<Client> &clients) // careful, there is also a method of RequestHandler called findClient()
{
	size_t i;
	for (i = 0; i < clients.size(); ++i)
	{
		if (clientFD == clients[i].getClientFD())
			return clients[i];
	}
	throw std::runtime_error("THIS MUST BE A NEW CLIENT OR A SERVER SOCKET");
	//std::cout << "RETURNING LAST CLIENT\n";
	return clients[--i]; // returning last one, should never happen!!
}

std::string sizeTToHexString(size_t value) {
	std::stringstream ss;
	ss << std::hex << value;
	return ss.str();
}

#include <sys/ioctl.h>

void printClientInfo(int clientFD, uint32_t events, std::vector<Client> clients) { // DEBUG MESSAGES
	Client &cli = findClient(clientFD, clients);
	//std::cout << BOLD_WHITE << "    Client FD: " << cli.getClientFD() << RESET << std::endl;
	if (events & EPOLLOUT)
		std::cout << BOLD_WHITE << "        EVENT: EPOLLOUT triggered" << RESET << std::endl;
	if (events & EPOLLIN)
		std::cout << BOLD_WHITE << "        EVENT: EPOLLIN triggered" << RESET << std::endl;
	if (events & EPOLLHUP)
		std::cout << BOLD_WHITE << "        EVENT: EPOLLHUP triggered" << RESET << std::endl;
	if (events & EPOLLERR)
		std::cout << BOLD_WHITE << "        EVENT: EPOLLERR triggered" << RESET << std::endl;
	
	std::cout << BOLD_WHITE << "       STATUS: " << cli.status << RESET << std::endl;
	if (cli.status == NEW) std::cout << BOLD_WHITE << std::setw(16) << cli.status << " means NEW\n";
	if (cli.status == RECIEVING) std::cout << BOLD_WHITE << std::setw(16) << cli.status << " means RECIEVING\n"; 
	if (cli.status == R_CHUNKS) std::cout << BOLD_WHITE << std::setw(16) << cli.status << " means RECIEVING CHUNKS\n"; 
	if (cli.status == SENDING) std::cout << BOLD_WHITE << std::setw(16) << cli.status << " means SENDING\n";
	if (cli.status == S_CHUNKS) std::cout << BOLD_WHITE << std::setw(16) << cli.status << " means SENDING CHUNKS\n";
	std::cout << RESET;
	std::cout << BOLD_WHITE << "   Client URI: " << cli.getRequest().path << RESET << std::endl;
	std::cout << BOLD_WHITE << "Client Method: " << cli.getRequest().method << RESET << std::endl;

	// New code to print the number of bytes in the fd
    int bytesAvailable = 0;
    if (ioctl(clientFD, FIONREAD, &bytesAvailable) != -1) {
        std::cout << BOLD_WHITE << "Bytes available in FD: " << bytesAvailable << RESET << std::endl;
    } else {
        std::cerr << BOLD_RED << "Error getting bytes available in FD" << RESET << std::endl;
    }
}

void Epoll::removeFD(int fd) {
    if (epoll_ctl(_epollFD, EPOLL_CTL_DEL, fd, NULL) == -1) {
        perror("epoll_ctl: remove");
        throw std::runtime_error("epoll_ctl: remove");
    }
    _clFDs.erase(std::remove(_clFDs.begin(), _clFDs.end(), fd), _clFDs.end()); // Remove from the list
}


#define GLOBAL_TIMEOUT 5000 // Global epoll timeout in milliseconds 5 seconds
#define CLIENT_TIMEOUT 10000 // Per-client timeout in milliseconds 10 seconds

void Epoll::handleEpollEvents(const std::vector<int> &serverSockets)
{
	std::vector<struct epoll_event> _events(MAX_EVENTS);
	int n;

	n = epoll_wait(_epollFD, _events.data(), MAX_EVENTS, GLOBAL_TIMEOUT); // waits for I/O events, blocks the calling thread if no events are currently available
	if (n == -1)
	{
		if (errno == EINTR)
			return;
		throw std::runtime_error("epoll_wait");
	}
	if (n == 0)//here is the logic of the ime out 
	{
		std::time_t now = std::time(NULL);
		std::cout << "now : " << now << std::endl;
		std::cout << "is timing out !!!!!!!" << std::endl;
		std::cout << "the size of clintes: " << _clients.size() << std::endl;
		for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end();)
		{
			std::cout << "ClientFD : " << it->getClientFD() << " and the time stamp of it : " << it->getClientTime() << std::endl;
			int clientTimeOut = now - it->getClientTime();
			std::cout << "clientTimeOut : " << clientTimeOut << std::endl;

			if (clientTimeOut > CLIENT_TIMEOUT / 1000)
			{
				std::cout << "client : " << it->getClientFD() << " timed out " << std::endl;
				killClient(it->getClientFD());
				close(it->getClientFD());
			}
			else
				++it;
		}
	}
	
	for (int i = 0; i < n; ++i)
	{
		
		if (std::find(_clFDs.begin(), _clFDs.end(), _events[i].data.fd) == _clFDs.end())
			_clFDs.push_back(_events[i].data.fd);
		if (_events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) // what do these errors/flags mean?
		{
			std::cout << "---------------------------------------------------\n";
			std::cerr << "Error on fd " << _events[i].data.fd << ": \n";
			if (_events[i].events & EPOLLERR)
				std::cerr << YELLOW_COLOR << "EPOLLERR\n" << RESET_COLOR;
			if (_events[i].events & EPOLLHUP)
				std::cerr << YELLOW_COLOR << "EPOLLHUP (hang-up)\n" << RESET_COLOR;
			if (_events[i].events & EPOLLRDHUP)
				std::cerr << YELLOW_COLOR << "EPOLLRDHUP (remote peer closed connection or shut down writing half on the connection)\n" << RESET_COLOR;
			std::cerr << std::endl;

			killClient(_events[i].data.fd);
			close(_events[i].data.fd);
			continue;
		}
		if (std::find(serverSockets.begin(), serverSockets.end(), _events[i].data.fd) != serverSockets.end())
		{
			handleConnection(_events[i].data.fd);
		}
		else if (_events[i].events & EPOLLIN)
		{
			handleData(_events[i].data.fd); // recieve Data & prep response
		}
		else if (_events[i].events & EPOLLOUT)
		{

			Client &client = findClient(_events[i].data.fd, _clients);

			std::string &remainingResBuffer = client.getResponse().body;
			std::string sendNow;
			if (client.getResponseBuffer().find("Transfer-Encoding: chunked") != std::string::npos && remainingResBuffer.size())
			{
				if (!client.getSentHeader())
				{
					client.status = S_CHUNKS;
					sendNow = client.getResponse().status + client.getResponse().location + client.getResponse().contentType + client.getResponse().transferEncoding + "\r\n";
					client.setSentHeader(true);
				}
				else
				{
					size_t chunkSize = std::min((size_t)SEND_CHUNK_SIZE, remainingResBuffer.size());
					std::string chunk = remainingResBuffer.substr(0, chunkSize);
					sendNow = sizeTToHexString(chunkSize) + "\r\n" + chunk + "\r\n";
					remainingResBuffer = remainingResBuffer.substr(chunkSize);
				}
			}
			else if (client.getResponseBuffer().find("Transfer-Encoding: chunked") != std::string::npos && !remainingResBuffer.size())
			{
				std::cout << "SENDING LAST CHUNK\n";
				sendNow = "0\r\n\r\n";
			}
			else
				sendNow = client.getResponseBuffer();
			send(_events[i].data.fd, sendNow.c_str(), sendNow.size(), 0);
			if (client.getResponseBuffer().find("Transfer-Encoding: chunked") != std::string::npos && sendNow != "0\r\n\r\n")
			{
				continue; // didn't send whole response, delay killing and closing
			}
			killClient(_events[i].data.fd);
			close(_events[i].data.fd);
		}
	}
}

// issue right now is that epoll_wait() doesn't report on a some of client fds, depending on chunk size, more or less of them

void Epoll::handleData(int client_fd)
{
	requestHandle.receiveData(client_fd, _clients);
	Client &clientB = findClient(client_fd, _clients);

	if (clientB.getIsAllRecieved())							 // we only go on here once we recieved the whole request
	{
		clientB.setResponse(sendData.sendResponse(clientB.getClientFD(), _servers, clientB.getRequest(), _epollFD));
		clientB.status = RECIEVED;
		struct epoll_event client_event;
		client_event.data.fd = clientB.getClientFD();
		client_event.events = EPOLLOUT;
		if (epoll_ctl(_epollFD, EPOLL_CTL_MOD, clientB.getClientFD(), &client_event) == -1)
		{
			close(clientB.getClientFD());
			throw std::runtime_error("in sendResponse(): epoll_ctl while MODIFYING client FD " + intToString(clientB.getClientFD()));
		}
		clientB.status = SENDING;
		std::string responseBuffer;
		responseBuffer = clientB.getResponse().status + clientB.getResponse().location + clientB.getResponse().contentType + clientB.getResponse().transferEncoding + clientB.getResponse().contentLength + "\r\n" + clientB.getResponse().body;
		clientB.setResponseBuffer(responseBuffer);
	}
}

void Epoll::handleConnection(int server_fd) // we add additionally to the server_socket_fds also the cliend fd to the interest list (set of fds to watch)
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
		// std::cout << "CLIENT FD: " << client_fd << std::endl;
		make_socket_non_blocking(client_fd);
		struct epoll_event client_event;
		client_event.data.fd = client_fd;
		client_event.events = EPOLLIN; // by default lvl triggered, so the client fds are all level triggered
		if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
		{
			close(client_fd);
			std::string err_msg("in handleConnection(): epoll_ctl() FAILED when ADDING client fd " + intToString(client_fd));
			throw std::runtime_error(err_msg);
		}
		Client newClient;
		newClient.setClientFD(client_fd); // creating new Client Object for the new client
		newClient.setClientTime(std::time(NULL));
		newClient.status = NEW;
		_clients.push_back(newClient);	  // and adding it to the _clients vector
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