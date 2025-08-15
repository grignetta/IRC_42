#!/bin/bash

echo "=== Testing protocol violations ==="

# Test commands without proper registration
{
    echo "JOIN #test"         # JOIN before registration
    echo "PRIVMSG #test :Hi"  # PRIVMSG before registration
    echo "MODE #test +i"      # MODE before registration
} | nc localhost 6667 &

# Test duplicate registration
{
    echo "PASS password123"
    echo "NICK duplicate"
    echo "USER duplicate 0 * :First registration"
    sleep 1
    echo "PASS password123"   # Duplicate PASS
    echo "USER duplicate 0 * :Second registration"  # Duplicate USER
} | nc localhost 6667 &

# Test wrong command order
{
    echo "USER wrongorder 0 * :Wrong Order"  # USER before PASS
    echo "NICK wrongorder"
    echo "PASS password123"
} | nc localhost 6667 &

wait
echo "Protocol violations test completed."