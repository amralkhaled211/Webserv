# NOTES FOR SERVER.CPP

## createSocket
- **Purpose**: Creates sockets 
- **Parameters**:
	-`AF_INET`: This indicates that the socket is intended for use with IPv4 addresses
	-`SOCK_STREAM`: We are using the TCP
- **Returns**:
	-`FD`: File descriptor of the socket
	-`-1`: on failur

## bindSocket
- **Purpose**: used to associate a socket with a specific local IP address and port number
- **Parameters**:
	- `serverSocket`: The file descriptor of the socket to be bound.
	- `addr`: A pointer to a `sockaddr` structure that contains the address to bind to.
	- `addrlen`: The size, in bytes, of the address structure pointed to by `addr`.


## make_socket_non_blocking
- **Purpose**: Makes the sockt_FD Non_Blocking 
- **Parameters**:
	- `sockfd`: The file descriptor of the socket.
	- `F_GETFL`: This gets the current file status flags of the socket
	- `O_NONBLOCK`: This is the flag thats goning to make the file Non_Blocking
	- `F_SETFL`: This sets the modified flags back to the socket.


