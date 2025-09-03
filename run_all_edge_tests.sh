#!/bin/bash

echo "=========================================="
echo "IRC Server Edge Case Testing Suite"
echo "=========================================="

# Check if server is running
if ! nc -z localhost 6667 2>/dev/null; then
    echo "Error: IRC server not running on localhost:6667"
    echo "Please start your server first: ./ircserv 6667 password123"
    exit 1
fi

echo "Server detected. Starting tests..."
sleep 2

# Run all tests
echo "1. Running malformed commands test..."
./test_malformed.sh
sleep 3

echo "2. Running buffer limits test..."
./test_buffer_limits.sh
sleep 3

echo "3. Running rapid connections test..."
./test_rapid_connections.sh
sleep 3

echo "4. Running special characters test..."
./test_special_chars.sh
sleep 3

echo "5. Running protocol violations test..."
./test_protocol_violations.sh
sleep 3

echo "=========================================="
echo "All edge case tests completed!"
echo "Check your server output for responses."
echo "=========================================="