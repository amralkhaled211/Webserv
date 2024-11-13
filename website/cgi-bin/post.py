#!/usr/bin/env python3

import cgi
import cgitb

# Enable CGI error reporting
cgitb.enable()

# Output the HTTP headers
print("Content-Type: text/html")
print()

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
name = form.getvalue('name')
email = form.getvalue('email')

# Output the HTML content
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>CGI POST Example</title>
</head>
<body>
    <h1>CGI POST Example</h1>
    <p>Name: {name}</p>
    <p>Email: {email}</p>
</body>
</html>
""")