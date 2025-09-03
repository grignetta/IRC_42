#!/bin/bash

echo "=== Testing rapid connections ==="

# Rapid connections that disconnect immediately
echo "Testing rapid disconnects..."
for i in {1..20}; do
    {
        echo "PASS password123"
        echo "NICK rapid$i"
        echo "USER rapid$i 0 * :Rapid User $i"
        sleep 0.1  # Brief pause
        # Connection will terminate here
    } | timeout 1 nc localhost 6667 &
    
    sleep 0.05  # Small delay between connections
done

wait
echo "Rapid connections test completed."