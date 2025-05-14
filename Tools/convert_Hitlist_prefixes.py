import argparse
import re
import ipaddress

def is_valid_ipv6(addr):
    """Check if an IPv6 address is valid"""
    try:
        ipaddress.IPv6Address(addr)
        return True
    except ipaddress.AddressValueError:
        return False

def convert_to_prefix(ip, prefix_length=48):
    """Convert an IPv6 address to a specified prefix length"""
    network = ipaddress.IPv6Network(f"{ip}/{prefix_length}", strict=False)
    return str(network)

def process_file(input_file, output_file):
    # Set to store valid prefixes
    prefixes = set()

    try:
        with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
            line_count = 0
            total_lines = 0

            # First pass to count total lines for progress tracking
            for line in infile:
                total_lines += 1
            infile.seek(0)  # Reset file pointer to the beginning

            print(f"Processing {total_lines} lines...")

            for line in infile:
                # Strip leading and trailing whitespace
                ip = line.strip()

                # Skip empty lines
                if not ip:
                    continue

                # Skip invalid IPv6 addresses
                if not is_valid_ipv6(ip):
                    print(f"Invalid IPv6 address: {ip}")
                    continue

                # Convert IPv6 address to prefix
                prefix = convert_to_prefix(ip, 48)  # Convert to /48 prefix

                # Add to the set (automatically removes duplicates)
                prefixes.add(prefix)

                line_count += 1
                if line_count % 1000000 == 0:  # Print progress every 1000 lines
                    print(f"Processed {line_count} lines out of {total_lines} ({(line_count / total_lines) * 100:.2f}%)")

            # Write unique prefixes to the output file
            for prefix in sorted(prefixes):
                outfile.write(f"{prefix}\n")

            # Output the number of prefixes generated
            print(f"Successfully generated {len(prefixes)} unique prefixes, saved to {output_file}")

    except FileNotFoundError:
        print(f"Error: File {input_file} not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    # Parse command line arguments
    parser = argparse.ArgumentParser(description="Convert IPv6 addresses to /48 prefixes.")
    parser.add_argument("input_file", help="Path to the input file containing IPv6 addresses")
    parser.add_argument("output_file", help="Path to the output file for the generated prefixes")
    args = parser.parse_args()

    # Process the file
    process_file(args.input_file, args.output_file)