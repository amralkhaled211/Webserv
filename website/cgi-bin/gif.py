#!/usr/bin/env python3

import os
import sys

# Set the path to the GIF image
gif_image = "/home/aismaili/Webserv/website/pics/captain.gif"

try:
    # Open the GIF image file in binary mode
    with open(gif_image, "rb") as image:
        # Get the size of the image file
        no_bytes = os.path.getsize(gif_image)
        
        # Output the HTTP headers
        print("Content-Type: image/gif")
        print(f"Content-Length: {no_bytes}")
        print()
        
        # Read and output the image file in chunks
        while True:
            data = image.read(1024)
            if not data:
                break
            sys.stdout.buffer.write(data)
except IOError:
    # If the image file cannot be opened, output an error message
    print("Content-Type: text/plain")
    print()
    print(f"Sorry! I cannot open the file {gif_image}!")