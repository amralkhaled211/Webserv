import cgi
import cgitb

# Enable CGI error reporting
cgitb.enable()

# Output the HTTP headers
print("Content-Type: text/html")
print()

# Generate a large HTML response
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Large Response</title></head>")
print("<body>")
print("<h1>This is a very large response</h1>")

# Repeat a simple pattern to generate a large response
for i in range(500000):  # Adjust the range to increase or decrease the size
    print(f"<p>Line {i}: This is a test line to generate a large response.</p>")

print("</body>")
print("</html>")