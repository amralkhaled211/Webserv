#!/usr/bin/env python3

import cgi
import cgitb
import sys
import io

# Enable CGI error reporting
cgitb.enable()

# Set the standard output to binary mode
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# Generate the response body
response_body = """
<!DOCTYPE html>
<html>
<head>
    <title>Simple CGI Response</title>
</head>
<body>
    <h1>Hello from CGI!</h1>
    <p>This is a simple CGI script response.</p>
</body>
</html>
"""

# Calculate content length
content_length = len(response_body)

# Output the HTTP headers
print("Content-Type: text/html")
print(f"Content-Length: {content_length}")
print("HTTP/1.1 200 OK")
print()

# Output the response body
print(response_body)