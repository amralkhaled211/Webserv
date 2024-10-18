#!/usr/bin/env python3

import os
import sys

def main():
    # Read the input from stdin
    input_data = sys.stdin.read()

    # Process the input data (for demonstration, we'll just echo it back)
    response_body = f"<html><body><h1>CGI Script Response</h1><p>{input_data}</p></body></html>"

    # Create the HTTP response
    response = (
        "Content-Type: text/html\r\n"
        f"Content-Length: {len(response_body)}\r\n"
        "\r\n"
        f"{response_body}"
    )

    # Output the response to stdout
    sys.stdout.write(response)
    sys.stdout.flush()

if __name__ == "__main__":
    main()