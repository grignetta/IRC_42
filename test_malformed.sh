#!/bin/bash

echo "=== Testing malformed commands ==="
echo "Starting server on localhost:6667..."

# Test completely invalid commands
echo "Testing invalid commands..."
{
    echo "INVALID_COMMAND"
    echo "GARBAGE_DATA_HERE"
    echo "123456789"
    echo ""  # Empty line
    echo "   "  # Spaces only
    echo "QUIT"
} | nc localhost 6667 &

# Test commands with missing parameters
echo "Testing commands with missing parameters..."
{
    echo "NICK"           # Missing nickname
    echo "USER"           # Missing all USER parameters
    echo "JOIN"           # Missing channel
    echo "MODE"           # Missing target
    echo "INVITE"         # Missing parameters
    echo "KICK"           # Missing parameters
    echo "PRIVMSG"        # Missing parameters
    echo "QUIT"
} | nc localhost 6667 &

wait
echo "Malformed commands test completed."