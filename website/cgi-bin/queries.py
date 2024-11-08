#!/usr/bin/env python3

import cgi
import cgitb

cgitb.enable()

# Get query string parameters
form = cgi.FieldStorage()

name = form.getvalue('name', 'Guest')
age = form.getvalue('age', 'Unknown')


print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Query String Response</title></head>")
print("<body>")
print(f"<h1>Hello, {name}!</h1>")
print(f"<p>You are {age} years old.</p>")
print("</body>")
print("</html>")