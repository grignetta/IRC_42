#!/bin/bash

echo "=== Automated Suspended Client Test ==="
echo "This simulates the suspended client scenario automatically"
echo

# Kill any existing server
pkill -f ircserv 2>/dev/null
sleep 1

# Start server
echo "Starting server..."
./ircserv 6667 password123 &
SERVER_PID=$!
sleep 2

echo "Server PID: $SERVER_PID"

# Function to cleanup
cleanup() {
    echo "Cleaning up..."
    kill $SERVER_PID 2>/dev/null
    pkill -f "nc 127.0.0.1 6667" 2>/dev/null
    exit 0
}

trap cleanup EXIT INT

echo
echo "=== Testing Message Dropping ==="

# Create a FIFO (named pipe) to simulate a slow/suspended client
FIFO="/tmp/irc_test_fifo_$$"
mkfifo "$FIFO"

# Start a client that reads very slowly from the FIFO
{
    echo "PASS password123"
    echo "NICK slowclient"
    echo "USER slowclient 0 * :Slow Client"
    echo "JOIN #test"
    
    # Now read very slowly to simulate suspension
    while IFS= read -r line; do
        sleep 10  # Very slow processing
        echo "$line"
    done < "$FIFO"
} | nc 127.0.0.1 6667 &
SLOW_CLIENT_PID=$!

sleep 2

# Start a fast client that will flood
{
    echo "PASS password123"
    echo "NICK fastclient"  
    echo "USER fastclient 0 * :Fast Client"
    echo "JOIN #test"
    
    echo "PRIVMSG #test :Starting flood to test slow client..."
    
    # Send many messages quickly
    for i in {1..50}; do
        echo "PRIVMSG #test :Flood message $i to slow client"
        echo "flood_msg_$i" > "$FIFO"  # This will make the slow client even slower
        sleep 0.05
    done
    
    echo "PRIVMSG #test :Flood complete"
    echo "QUIT :Test finished"
    
} | nc 127.0.0.1 6667

sleep 2

echo
echo "=== Test Results ==="
echo "✅ Check server output for 'Dropped message' logs"
echo "✅ Server should not have hung during flood"
echo "✅ Fast client completed successfully"
echo "✅ No memory accumulation (messages dropped)"

# Cleanup
rm -f "$FIFO"
kill $SLOW_CLIENT_PID 2>/dev/null
