
use strict;
use warnings;

print "Content-Type: text/html\n\n";
print "<!DOCTYPE html>\n";
print "<html>\n";
print "<head><title>Environment Variables</title></head>\n";
print "<body>\n";
print "<h1>Environment Variables</h1>\n";
print "<ul>\n";

foreach my $key (sort keys %ENV) {
    print "<li><strong>$key:</strong> $ENV{$key}</li>\n";
}

print "</ul>\n";
print "</body>\n";
print "</html>\n";