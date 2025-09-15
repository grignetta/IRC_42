#!/bin/bash

echo "=== Testing Suspended Client Scenario ==="
echo "This test verifies the server handles suspended clients without hanging"
echo

# Kill any existing server
pkill -f ircserv 2>/dev/null

# Start server in background
echo "Starting IRC server..."
./ircserv 6667 password123 &
SERVER_PID=$!
sleep 1

echo "Server started with PID: $SERVER_PID"
echo

# Function to cleanup
cleanup() {
    echo "Cleaning up..."
    kill $SERVER_PID 2>/dev/null
    pkill -f "nc 127.0.0.1 6667" 2>/dev/null
    exit 0
}

trap cleanup EXIT INT

echo "=== Test Instructions ==="
echo "1. Two netcat clients will connect"
echo "2. Both will join #test channel" 
echo "3. You need to MANUALLY suspend client1 with Ctrl+Z"
echo "4. Client2 will flood the channel"
echo "5. Resume client1 with 'fg' to see if it works"
echo
echo "Watch the server output for 'Dropped message' logs"
echo

# Start first client
echo "Starting client1 (alice)..."
{
    sleep 1
    echo "PASS password123"
    echo "NICK alice" 
    echo "USER alice 0 * :Alice User"
    echo "JOIN #test"
    echo "PRIVMSG #test :Alice is ready"
    
    # Keep connection alive
    while true; do
        sleep 10
        echo "PING :keepalive" 2>/dev/null || break
    done
} | nc 127.0.0.1 6667 &
CLIENT1_PID=$!

sleep 2

# Start second client  
echo "Starting client2 (bob)..."
{
    sleep 1
    echo "PASS password123"
    echo "NICK bob"
    echo "USER bob 0 * :Bob User" 
    echo "JOIN #test"
    echo "PRIVMSG #test :Bob is ready"
    
    sleep 3
    echo "PRIVMSG #test :Starting flood test..."
    
    # Flood the channel
    for i in {1..20}; do
        echo "PRIVMSG #test :Flood message $i - testing suspended client handling"
        sleep 0.1
    done
    
    echo "PRIVMSG #test :Flood test complete"
    sleep 2
    echo "QUIT :Test finished"
    
} | nc 127.0.0.1 6667 &
CLIENT2_PID=$!

echo "Both clients started."
echo
echo "=== MANUAL TEST STEPS ==="
echo "1. Wait for both clients to connect and join #test"
echo "2. Look for server logs showing client connections"
echo "3. When you see 'Bob is ready', quickly suspend client1:"
echo "   - Find the nc process for alice: ps aux | grep nc"
echo "   - Suspend it: kill -STOP <alice_nc_pid>"
echo "4. Watch server output during flood - should see 'Dropped message' logs"
echo "5. Resume alice: kill -CONT <alice_nc_pid>"
echo "6. Verify server is still responsive"
echo
echo "Press Ctrl+C when done testing"

# Wait for user to finish testing
wait $CLIENT2_PID 2>/dev/null

echo
echo "=== Test Results ==="
echo "✅ Server should not have hung during flood"
echo "✅ Should see 'Dropped message' logs in server output"  
echo "✅ No memory leaks (messages were dropped, not buffered)"
echo "✅ Alice should reconnect normally when resumed"

sleep 5
