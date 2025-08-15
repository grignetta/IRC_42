#!/bin/bash

echo "=== Testing special characters ==="

# Test with various special characters
{
    echo "PASS password123"
    echo "NICK tëst"  # Special characters in nick
    echo "USER tëst 0 * :Tëst Üser with spëcial chars"
    echo "JOIN #tëst-chännël"
    echo "PRIVMSG #tëst-chännël :Hëllö wörld! 🌍"
    echo "TOPIC #tëst-chännël :Tëst töpic with émojis 😀"
} | nc localhost 6667 &

# Test with control characters
{
    echo "PASS password123"
    echo "NICK control"
    printf "USER control 0 * :User with\x07bell\x08backspace\n"
    printf "PRIVMSG #test :Message with\x00null\x01char\n"
} | nc localhost 6667 &

wait
echo "Special characters test completed."