# Compare Files - Fast Using C++20

This project is designed to compare two large text files and highlight the differences between them. It uses multi-threading to efficiently process large files and marks the differences line by line.

## Project Structure

- **main.cpp**: The main program that compares two files and prints the differences with markers.
- **CMakeLists.txt**: CMake build configuration for the project.
- **generate_test_files.py**: A Python script to generate large test files for use with the comparison program.
- **compare_performance.sh**: A Bash script to benchmark the performance of the custom comparison tool against the `diff` command.

## Requirements

- C++20 compiler
- CMake 3.12 or higher
- Python 3.6 or higher
- Bash shell (for performance testing)

## Build Instructions

1. Clone the repository or download the project files.
2. Run the following commands to build the project:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```
   This will generate the `cmp` executable.

## Usage

### Compare Files

To compare two text files:
```bash
./cmp <file1> <file2>
```

Where:
- `file1`: The path to the first file.
- `file2`: The path to the second file.

Example:
```bash
./cmp large_file1.txt large_file2.txt
```

The program will print the differences between the two files, marking the lines that are different.

### Generate Test Files

To generate large test files, use the `generate_test_files.py` script:
```bash
python3 generate_test_files.py <diff_percentage> <chars_to_change> [--num_lines <num_lines>] [--line_length <line_length>]
```

- `diff_percentage`: The percentage of lines that should differ between the two generated files.
- `chars_to_change`: The number of characters to change in each different line.
- `--num_lines`: The total number of lines in each file (default: 1,000,000).
- `--line_length`: The length of each line (default: 100 characters).

Example:
```bash
python3 generate_test_files.py 5 10 --num_lines 500000 --line_length 80
```

This will generate two files:
- `large_file1.txt`
- `large_file2.txt` (with 5% of the lines differing from `large_file1.txt` and 10 characters changed per different line).

### Output

The comparison program will output the differences in the following format:
- `+` indicates a line from `file1` that differs.
- `-` indicates a line from `file2` that differs.
- A `^` marker shows the specific character positions where the differences occur.

## Example

```bash
./cmp large_file1.txt large_file2.txt
```

Output:
```
-  1: hello world
+  1: hello_world
           ^
Total differences: 1
```

## Performance Comparison

You can compare the performance of this tool against the Linux `diff` command using the provided `compare_performance.sh` script.

Run the following command:
```bash
./compare_performance.sh large_file1.txt large_file2.txt
```

### Sample Results:

```
Testing custom cmp tool...
Custom cmp tool time: .311590268 seconds
Testing diff command...
diff command time: .508468469 seconds
```

As seen from this comparison, the custom tool (`cmp`) is faster than the standard `diff` command for large files.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
