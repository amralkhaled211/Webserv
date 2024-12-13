#!/usr/bin/env python3

import cgi
import cgitb

# Enable CGI error reporting (useful for debugging)
cgitb.enable()

# Set the content type to HTML
print("Content-Type: text/html\n")

# Create an instance of the FieldStorage class to handle the form data
form = cgi.FieldStorage()

# Check if the form has data and retrieve values
if "name" in form and "email" in form:
    name = form.getvalue("name")
    email = form.getvalue("email")
    
    # Process the data (in this case, just display it back to the user)
    print("<html>")
    print("<head><title>Form Data Submitted</title></head>")
    print("<body>")
    print("<h2>Form Data Received</h2>")
    print(f"<p><strong>Name:</strong> {name}</p>")
    print(f"<p><strong>Email:</strong> {email}</p>")
    print("</body>")
    print("</html>")
else:
    # If no data was received, show an error message or request data again
    print("<html>")
    print("<head><title>Error</title></head>")
    print("<body>")
    print("<h2>Error: Form not submitted correctly!</h2>")
    print("<p>Please ensure that you have filled in both the 'name' and 'email' fields.</p>")
    print("</body>")
    print("</html>")