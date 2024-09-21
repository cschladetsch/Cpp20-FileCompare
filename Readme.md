# Fast File Comparison Tool

This project provides high-performance file comparison tools implemented in both C++20 and Rust. These tools are designed to efficiently compare large text files and highlight the differences between them, significantly outperforming the standard `diff` command.

## Project Structure

- **C++ Implementation**:
  - `main.cpp`: The main C++ program for file comparison.
  - `CMakeLists.txt`: CMake build configuration for the C++ project.
- **Rust Implementation**:
  - `src/main.rs`: The main Rust program for file comparison.
  - `Cargo.toml`: Rust project and dependency configuration.
- **Shared Resources**:
  - `generate_test_files.py`: Python script to generate large test files.
  - `compare_performance.sh`: Bash script to benchmark all implementations.

## Requirements

- C++20 compiler (for C++ version)
- CMake 3.12 or higher (for C++ version)
- Rust compiler (for Rust version)
- Python 3.6 or higher (for test file generation)
- Bash shell (for performance testing)

## Build Instructions

### C++ Version

1. Navigate to the C++ project directory.
2. Run the following commands:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```
   This will generate the `cmp` executable.

### Rust Version

1. Navigate to the Rust project directory.
2. Run the following command:
   ```bash
   cargo build --release
   ```
   This will generate the `fast_file_compare` executable in the `target/release` directory.

## Usage

### Comparing Files

For the C++ version:
```bash
./cmp <file1> <file2>
```

For the Rust version:
```bash
./target/release/fast_file_compare <file1> <file2>
```

Where:
- `file1`: Path to the first file.
- `file2`: Path to the second file.

### Generating Test Files

Use the Python script to generate large test files:
```bash
python3 generate_test_files.py <diff_percentage> <chars_to_change> [--num_lines <num_lines>] [--line_length <line_length>]
```

Parameters:
- `diff_percentage`: Percentage of lines that should differ between the files.
- `chars_to_change`: Number of characters to change in each different line.
- `--num_lines`: Total number of lines in each file (default: 1,000,000).
- `--line_length`: Length of each line (default: 100 characters).

Example:
```bash
python3 generate_test_files.py 5 10 --num_lines 500000 --line_length 80
```

This generates `large_file1.txt` and `large_file2.txt` with the specified differences.

## Output Format

Both implementations output differences in the following format:
- `+` indicates a line from `file1` that differs.
- `-` indicates a line from `file2` that differs.
- `^` markers show specific character positions where differences occur.

Example output:
```
-  1: hello world
+  1: hello_world
           ^
Total differences: 1
```

## Performance Comparison

Use the `compare_performance.sh` script to benchmark all implementations:
```bash
./compare_performance.sh large_file1.txt large_file2.txt
```

### Sample Results

```
Testing custom cmp tool...
Custom cmp tool time: .001919209 seconds
Testing diff command...
diff command time: .563816515 seconds
Testing Rust implementation...
Rust implementation time: .002148358 seconds

Percentage differences:
Custom cmp tool is 29277.00% faster than diff
Custom cmp tool is 11.00% faster than Rust implementation
Rust implementation is 26144.00% faster than diff
```

### Key Findings

1. Both custom implementations (C++ and Rust) significantly outperform the standard `diff` command.
2. The C++ implementation shows a slight performance edge over the Rust implementation.
3. Performance improvements:
   - C++ version: ~29,277% faster than `diff`
   - Rust version: ~26,144% faster than `diff`
   - C++ version: ~11% faster than Rust version

These results demonstrate the efficiency of both implementations, with the C++ version having a slight advantage in raw performance.

## Implementation Details

### C++ Version
- Uses memory mapping for efficient file I/O.
- Employs multi-threading for parallel processing of file chunks.
- Utilizes C++20 features for optimal performance.

### Rust Version
- Also uses memory mapping (via the `memmap2` crate).
- Leverages Rust's safety features and zero-cost abstractions.
- Uses the `rayon` crate for parallel processing.

Both versions prioritize performance while maintaining accuracy in file comparison.

## Contributing

Contributions to improve either implementation are welcome. Please submit pull requests with any enhancements or bug fixes.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
