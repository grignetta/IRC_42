#!/bin/bash

echo "=== Testing special characters ==="

# Test with VALID special characters (according to IRC RFC)
{
    echo "PASS password123"
    echo "NICK test_user"  # Underscore is valid
    echo "USER test_user 0 * :Test User"
    echo "JOIN #test-channel"  # Hyphen is valid in channels
    echo "PRIVMSG #test-channel :Regular ASCII message"
    echo "TOPIC #test-channel :Topic with [brackets] and |pipes|"
    echo "QUIT :Leaving"
} | nc localhost 6667 &

# Test boundary cases
{
    echo "PASS password123"  
    echo "NICK a"  # Very short nick
    echo "USER a 0 * :Short"
    echo "JOIN #a"
    echo "QUIT :Short test"
} | nc localhost 6667 &

wait
echo "Special characters test completed."