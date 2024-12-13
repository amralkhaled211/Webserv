VIDEO_PATH="/home/aismaili/Webserv/website/pics/NPC.mp4"

# Check if the video file exists
if [ ! -f "$VIDEO_PATH" ]; then
    echo "Content-Type: text/plain"
    echo ""
    echo "Error: Video file not found."
    exit 1
fi

# Get the size of the video file
VIDEO_SIZE=$(stat -c%s "$VIDEO_PATH")

# Output the HTTP headers
echo "Content-Type: video/mp4"
echo "Content-Length: $VIDEO_SIZE"
echo ""

# Send the video file
cat "$VIDEO_PATH"