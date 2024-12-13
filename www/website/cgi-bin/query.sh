echo "Content-Type: text/html"
echo ""

# Extract query string
QUERY_STRING=$(echo "$QUERY_STRING" | sed 's/+/ /g; s/%/\\x/g')

# Parse query string
NAME=$(echo "$QUERY_STRING" | awk -F= '{print $2}')

# Default to "Guest" if no name is provided
if [ -z "$NAME" ]; then
    NAME="Guest"
fi

echo "<!DOCTYPE html>"
echo "<html>"
echo "<head><title>GET Example</title></head>"
echo "<body>"
echo "<h1>Hello, $NAME!</h1>"
echo "</body>"
echo "</html>"