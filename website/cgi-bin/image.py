#!/usr/bin/env python3

import os
from PIL import Image
import cgi
import cgitb

# Enable CGI error reporting
cgitb.enable()

# Set the image file path
image_path = "/home/aszabo/Docs/Webserv/website/pics/aszabo.jpg"

# Check if the image file exists
if not os.path.isfile(image_path):
    print("Content-Type: text/html")
    print()
    print("<html><body><h1>Image not found</h1></body></html>")
    exit(1)

# Open the image and get its dimensions
image = Image.open(image_path)
image_width, image_height = image.size

# Output the HTTP headers
print("Content-Type: text/html")
print()

# Output the HTML content
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Image Information</title>
</head>
<body>
    <h1>Image Information</h1>
    <p>File: aszabo.jpg</p>
    <p>Dimensions: {image_width}x{image_height}</p>
    <img src="/website/pics/aszabo.jpg" alt="aszabo.jpg">
</body>
</html>
""")