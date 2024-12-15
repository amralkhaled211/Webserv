<?php
$video_path = "/home/aismaili/Webserv/www/website/pics/NPC.mp4";

// Check if the video file exists
if (!file_exists($video_path)) {
    header("Content-Type: text/plain");
    echo "Error: Video file not found.";
    exit(1);
}

// Get the size of the video file
$video_size = filesize($video_path);

// Output the HTTP headers
echo("Content-Type: video/mp4\n");
header("Content-Length: " . $video_size);

// Send the video file
readfile($video_path);
?>