VIDEO_PATH="/home/aszabo/Docs/Webserv/website/pics/NPC.mp4"

# Check if the video file exists
if [ ! -f "$VIDEO_PATH" ]; then
    echo "Content-Type: text/plain"
    echo ""
    echo "Error: Video file not found."
    exit 1
fi

# Send the video file
echo "Content-Type: video/mp4"
echo "Content-Length: $(stat -c%s "$VIDEO_PATH")"
echo ""
cat "$VIDEO_PATH"