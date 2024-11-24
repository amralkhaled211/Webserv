#!/usr/bin/env python3

import os
from moviepy.editor import VideoFileClip
import cgi
import cgitb

# Enable CGI error reporting
cgitb.enable()

# Set the video file path
video_path = "/home/aismaili/Webserv/website/pics/10min.mp4"

# Check if the video file exists
if not os.path.isfile(video_path):
    print("Content-Type: text/html")
    print()
    print("<html><body><h1>Video not found</h1></body></html>")
    exit(1)

# Open the video and get its duration and dimensions
video = VideoFileClip(video_path)
video_duration = video.duration
video_width, video_height = video.size

# Output the HTTP headers
print("Content-Type: text/html")
print()

# Output the HTML content
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>Video Information</title>
</head>
<body>
    <h1>Video Information</h1>
    <p>File: 10min.mp4</p>
    <p>Dimensions: {video_width}x{video_height}</p>
    <p>Duration: {video_duration:.2f} seconds</p>
    <video width="{video_width}" height="{video_height}" controls>
        <source src="/website/pics/10min.mp4" type="video/mp4">
        Your browser does not support the video tag.
    </video>
</body>
</html>
""")