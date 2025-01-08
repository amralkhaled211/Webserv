#!/bin/bash

# Check if an argument is provided
if [ -z "$1" ]; then
    echo "No argument provided. Running server normally..."
    ./webserv conf_files/default.conf
    exit 0
fi

# Run the webserv based on the argument
if [ "$1" == "valgrind" ]; then
    echo "Running webserv with Valgrind..."
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./webserv conf_files/default.conf
elif [ "$1" == "normal" ]; then
    echo "Running webserv normally..."
    ./webserv conf_files/default.conf
else
    echo "Invalid argument. Usage: $0 [normal|valgrind]"
    exit 1
fi

