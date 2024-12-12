#!/bin/bash

# Enable unlimited core dump size for the current session
ulimit -c unlimited
echo "Core dump size set to unlimited."

# Configure core dump file naming pattern
CORE_DUMP_DIR="." # Replace with your desired directory
CORE_PATTERN="$CORE_DUMP_DIR/core.%e.%p"
sudo mkdir -p "$CORE_DUMP_DIR"  # Ensure the directory exists
sudo chmod 777 "$CORE_DUMP_DIR" # Set permissions for all users to write core dumps
echo "$CORE_PATTERN" | sudo tee /proc/sys/kernel/core_pattern

echo "Core dump pattern set to: $CORE_PATTERN"

