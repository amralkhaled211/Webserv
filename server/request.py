import socket

host = "localhost"
port = 8081

request = """ / HTTP/1.1\r
Host: localhost:8081\r
\r
"""

with socket.create_connection((host, port)) as s:
    s.sendall(request.encode())
    response = s.recv(4096)
    print(response.decode())
