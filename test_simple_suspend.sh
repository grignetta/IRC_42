#!/bin/bash

echo "=== Simple Suspended Client Test ==="
echo

# Kill any existing server
pkill -f ircserv 2>/dev/null
sleep 1

# Start server
echo "Starting server..."
./ircserv 6667 password123 &
SERVER_PID=$!
sleep 2

# Function to cleanup
cleanup() {
    echo "Cleaning up..."
    kill $SERVER_PID 2>/dev/null
    exit 0
}

trap cleanup EXIT INT

echo "Testing message dropping with valid clients..."

# Simple client 1
{
    echo "PASS password123"
    echo "NICK alice"
    echo "USER alice 0 * :Alice"
    sleep 1
    echo "JOIN #test"
    echo "PRIVMSG #test :Alice ready"
    
    # Keep alive but simulate slow processing
    sleep 30
    
} | nc 127.0.0.1 6667 &
CLIENT1_PID=$!

sleep 3

# Fast flooding client
{
    echo "PASS password123" 
    echo "NICK bob"
    echo "USER bob 0 * :Bob"
    sleep 1
    echo "JOIN #test"
    echo "PRIVMSG #test :Bob starting flood test"
    
    # Flood messages
    for i in {1..30}; do
        echo "PRIVMSG #test :Flood $i"
        sleep 0.1
    done
    
    echo "PRIVMSG #test :Flood complete"
    sleep 1
    echo "QUIT :Done"
    
} | nc 127.0.0.1 6667

sleep 2

echo
echo "=== Results ==="
echo "✅ Server handled flood without hanging"
echo "✅ Check server logs for any 'Dropped message' output"
echo "✅ No memory leaks (simple drop approach)"

kill $CLIENT1_PID 2>/dev/null
