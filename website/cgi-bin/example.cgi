
cat << EOF
<!DOCTYPE html>
<html>
<head>
    <title>CGI Script Example</title>
</head>
<body>
    <h1>Hello from CGI Script!</h1>
    <p>This is a simple CGI script written in Bash.</p>
    <p>Current Date and Time: $(date)</p>
    <p>Query String: $QUERY_STRING</p>
</body>
</html>
EOF