use strict;
use warnings;

my $video_path = "/home/aismaili/Webserv/website/pics/NPC.mp4";

# Check if the video file exists
unless (-e $video_path) {
    print "Content-Type: text/plain\n\n";
    print "Error: Video file not found.";
    exit 1;
}

# Get the size of the video file
my $video_size = -s $video_path;

# Output the HTTP headers
print "Content-Type: video/mp4\n";
print "Content-Length: $video_size\n\n";

# Send the video file
open my $video_file, '<', $video_path or die "Cannot open video file: $!";
binmode $video_file;
print while <$video_file>;
close $video_file;