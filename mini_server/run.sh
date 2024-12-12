#!/bin/bash

# Check if an argument is provided
if [ -z "$1" ]; then
    echo "No argument provided. Running server normally..."
    ./Server conf_files/abe.conf
    exit 0
fi

# Run the server based on the argument
if [ "$1" == "valgrind" ]; then
    echo "Running server with Valgrind..."
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./Server conf_files/abe.conf #--log-file=valgrind.log 
elif [ "$1" == "normal" ]; then
    echo "Running server normally..."
    ./Server conf_files/abe.conf
else
    echo "Invalid argument. Usage: $0 [normal|valgrind]"
    exit 1
fi

