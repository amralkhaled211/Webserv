#include "server.hpp"

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


Server::Server()
{
	serverSocket = 0;
	clientSocket = 0;
}

Server::~Server()
{
	close(serverSocket);
	close(clientSocket);
	close(epoll_fd);
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
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);// Bind to port 8080
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Bind to any available address

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

	struct epoll_event event;
    event.data.fd = serverSocket;
    event.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) == -1)
	{
		throw std::runtime_error("epoll_ctl");
    }

	std::vector<struct epoll_event> events(MAX_EVENTS);

	while (isServerRunning())
	{
		int n = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
		if (n == -1)
		{
			if (errno == EINTR)
			{
				continue;
			}
			throw std::runtime_error("epoll_wait");
		}
		int i = 0;
		while (i < n)
		{
			if (events[i].events & (EPOLLERR))
			{
               // Handle error
               close(events[i].data.fd);
			   std::cout << "Connection closed" << std::endl;
			   throw std::runtime_error("epoll error_event");
            }
			//more details on the error
		
			else if (serverSocket == events[i].data.fd)
			{
				while (true)
				{
					sockaddr_in client_addr;
					socklen_t client_addr_len = sizeof(client_addr);
					int client_fd = accept(serverSocket, (struct sockaddr *)&client_addr, &client_addr_len);
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
						throw std::runtime_error("epoll_ctl");
					}
					std::cout << "Connection accepted" << std::endl;
				}
			}
			else
			{
				std::cout << "Data received" << std::endl;
				clientSocket = events[i].data.fd;
				receiveData();
				parseRequest();
				sendResponse();
			}
			i++;
		}
	}
}

void Server::receiveData()
{
	char buffer[1024] = {0};
	int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesReceived < 0)
    {
		close(clientSocket);
        throw std::runtime_error("Receiving failed");
    }
	this->buffer = buffer;
	//std::cout << "Message from client: " << buffer << std::endl;
}