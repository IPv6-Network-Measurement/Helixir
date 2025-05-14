#!/bin/bash

# Define parameters
INTERFACE_NAME=""
SOURCE_MAC=""
SOURCE_IP=""
GATEWAY_MAC=""
INPUT_FILENAME="input/Hitlist_prefixes.txt"
OUTPUT_DIR="output"
OUTPUT_FILENAME="$OUTPUT_DIR/$(date +'%Y%m%d_%H%M%S').log"

# Ensure the output directory exists
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir -p "$OUTPUT_DIR"
    echo "Created directory: $OUTPUT_DIR"
fi

# Check if the executable exists
if [ ! -f "bin/main" ]; then
    echo "Executable not found. Please compile the project first using 'make'."
    exit 1
fi

# Run the program
echo "Running the program..."
sudo bin/main "$INTERFACE_NAME" "$SOURCE_MAC" "$SOURCE_IP" "$GATEWAY_MAC" "$INPUT_FILENAME" "$OUTPUT_FILENAME"
