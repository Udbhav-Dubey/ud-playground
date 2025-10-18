#!/bin/bash

# Compile the server
echo "Compiling maze_server.c++..."
g++ maze_server.c++ -o maze_server
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run the compiled server in the background
echo "Running maze_server..."
./maze_server &
SERVER_PID=$!

# Wait a bit for the server to start
sleep 2

# Open the HTML frontend in Firefox
echo "Opening maze_frontend_client.html in Firefox..."
firefox maze_frontend_client.html &

# Wait for server to end (optional, keeps script alive until server stops)
wait $SERVER_PID

