import random
import string
import sys
import argparse

def generate_random_line(length):
    return ''.join(random.choices(string.ascii_letters + string.digits + string.punctuation + ' ', k=length))

def modify_string(s, num_chars):
    chars = list(s)
    positions = random.sample(range(len(s)), num_chars)
    for pos in positions:
        chars[pos] = random.choice(string.ascii_letters + string.digits + string.punctuation + ' ')
    return ''.join(chars)

def create_large_file(filename, num_lines, line_length):
    with open(filename, 'w') as f:
        for _ in range(num_lines):
            line = generate_random_line(line_length)
            f.write(line + '\n')

def create_modified_file(original_filename, modified_filename, diff_percentage, chars_to_change):
    with open(original_filename, 'r') as original, open(modified_filename, 'w') as modified:
        for line in original:
            if random.random() < diff_percentage / 100:
                # Modify the line
                modified_line = modify_string(line.strip(), chars_to_change)
                modified.write(modified_line + '\n')
            else:
                modified.write(line)

def parse_arguments():
    parser = argparse.ArgumentParser(description='Generate large test files with controlled differences.')
    parser.add_argument('diff_percentage', type=float, help='Percentage of lines to be different (0-100)')
    parser.add_argument('chars_to_change', type=int, help='Number of characters to change in each different line')
    parser.add_argument('--num_lines', type=int, default=1_000_000, help='Number of lines in each file (default: 1,000,000)')
    parser.add_argument('--line_length', type=int, default=100, help='Length of each line (default: 100)')
    return parser.parse_args()

def main():
    args = parse_arguments()

    if not 0 <= args.diff_percentage <= 100:
        print("Error: diff_percentage must be between 0 and 100")
        sys.exit(1)

    if args.chars_to_change > args.line_length:
        print(f"Error: chars_to_change ({args.chars_to_change}) cannot be greater than line_length ({args.line_length})")
        sys.exit(1)

    print("Generating first large file...")
    create_large_file('large_file1.txt', args.num_lines, args.line_length)

    print(f"Generating second large file with {args.diff_percentage}% of lines modified...")
    create_modified_file('large_file1.txt', 'large_file2.txt', args.diff_percentage, args.chars_to_change)

    print("Files generated successfully!")
    print(f"Each file is approximately {(args.num_lines * (args.line_length + 1)) / (1024 * 1024):.2f} MB")
    print(f"Approximately {args.diff_percentage}% of lines in large_file2.txt are different from large_file1.txt")
    print(f"Each different line has {args.chars_to_change} characters changed")

if __name__ == "__main__":
    main()
