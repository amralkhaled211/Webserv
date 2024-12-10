#!/usr/bin/env python3

import os
import sys

# Set the path to the image
image_path = "/home/aszabo/Docs/Webserv/website/pics/cap.gif"

try:
    # Open the image file in binary mode
    with open(image_path, "rb") as image:
        # Get the size of the image file
        image_data = image.read()
        content_length = len(image_data)
        
        # Output the HTTP headers
        print("Content-Type: image/gif")
        print("Status: 200 OK")
        print()
        
        # Output the image data
        sys.stdout.buffer.write(image_data)
except FileNotFoundError:
    # Output the HTTP headers for a 404 Not Found response
    response_body = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>404 Not Found</title>
    </head>
    <body>
        <h1>404 Not Found</h1>
        <p>The requested image was not found on this server.</p>
    </body>
    </html>
    """
    content_length = len(response_body)
    
    print("Content-Type: text/html")
    print(f"Content-Length: {content_length}")
    print("Status: 404 Not Found")
    print()
    
    # Output the response body
    print(response_body)
except IOError:
    # Output the HTTP headers for a 500 Internal Server Error response
    response_body = """
    <!DOCTYPE html>
    <html>
    <head>
        <title>500 Internal Server Error</title>
    </head>
    <body>
        <h1>500 Internal Server Error</h1>
        <p>There was an error processing your request.</p>
    </body>
    </html>
    """
    content_length = len(response_body)
    
    print("Content-Type: text/html")
    print(f"Content-Length: {content_length}")
    print("Status: 500 Internal Server Error")
    print()
    
    # Output the response body
    print(response_body)