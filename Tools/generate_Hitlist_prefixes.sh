#!/bin/bash

# Define the input directory
INPUT_DIR="input"

# Ensure the input directory exists
mkdir -p "$INPUT_DIR"

# Function to check if the previous command succeeded
check_error() {
    if [ $? -ne 0 ]; then
        echo "Error: $1"
        echo "Exiting script."
        exit 1
    fi
}

# Download the latest IPv6 Hitlist data
echo "Downloading latest IPv6 Hitlist data..."
wget -P "$INPUT_DIR" https://alcatraz.net.in.tum.de/ipv6-hitlist-service/open/responsive-addresses.txt.xz
check_error "Failed to download responsive-addresses.txt.xz."

# Decompress the downloaded file
echo "Decompressing the downloaded file..."
xz -d "$INPUT_DIR/responsive-addresses.txt.xz"
check_error "Failed to decompress responsive-addresses.txt.xz."

# Convert the data to Hitlist prefixes
echo "Converting data to Hitlist prefixes... This may take a few minutes."
python3 Tools/convert_Hitlist_prefixes.py "$INPUT_DIR/responsive-addresses.txt" "$INPUT_DIR/Hitlist_prefixes.txt"
check_error "Failed to convert responsive-addresses.txt to Hitlist_prefixes.txt."

# Clean up intermediate files
echo "Cleaning up intermediate files..."
rm "$INPUT_DIR/responsive-addresses.txt"
check_error "Failed to remove intermediate file responsive-addresses.txt."

echo "Hitlist_prefixes.txt has been successfully generated in $INPUT_DIR/Hitlist_prefixes.txt"