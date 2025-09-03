#!/bin/bash

echo "=== Testing buffer limits ==="

# Create very long lines (over 510 chars)
echo "Testing oversized messages..."

# Generate 600-character string
long_string=$(python3 -c "print('A' * 600)")

{
    echo "PASS password123"
    echo "NICK testuser"
    echo "USER testuser 0 * :Test User"
    echo "JOIN #test"
    echo "PRIVMSG #test :$long_string"  # This should be truncated/rejected
} | nc localhost 6667 &

{
    echo "PASS password123"
    echo "NICK testuser1"
    echo "USER testuser1 0 * :Test User"
    echo "JOIN #test"
    echo "PRIVMSG #test :test message"  # This should be truncated/rejected
} | nc localhost 6667 &

{
    echo "PASS password123"
    echo "NICK newUser"
    echo "USER newUser 0 * :Test User"
    echo "JOIN #test"
    echo "PRIVMSG #test :test message"  # This should be truncated/rejected
} | nc localhost 6667 &

# Test gradual buffer filling
echo "Testing gradual buffer overflow..."
{
    echo "PASS password123"
    echo "NICK gradual"
    echo "USER gradual 0 * :Gradual User"
    echo "JOIN #test"
    # Send many short messages rapidly
    for i in {1..100}; do
        echo "PRIVMSG #test :Message $i"
    done
} | nc localhost 6667 &

wait
echo "Buffer limits test completed."