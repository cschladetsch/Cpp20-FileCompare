# Fast File Comparison Tool

This project provides high-performance file comparison tools implemented in both C++20 and Rust. These tools are designed to efficiently compare large text files and highlight the differences between them, significantly outperforming the standard `diff` command.

## Project Structure

- **C++ Implementation**:
  - `main.cpp`: The main C++ program for file comparison.
  - `CMakeLists.txt`: CMake build configuration for the C++ project.
- **Rust Implementation**:
  - `rust_compare/src/main.rs`: The main Rust program for file comparison.
  - `rust_compare/Cargo.toml`: Rust project and dependency configuration.
- **Shared Resources**:
  - `generate_test_files.py`: Python script to generate large test files.
  - `compare_performance.sh`: Bash script to benchmark all implementations.
  - `br`: Bash script to build both implementations and run tests.

## Requirements

- C++20 compiler (for C++ version)
- CMake 3.12 or higher (for C++ version)
- Rust compiler (for Rust version)
- Python 3.6 or higher (for test file generation)
- Bash shell (for performance testing and build script)

## Build Instructions

Use the `br` script to build both C++ and Rust implementations, generate test files, and run performance comparisons:

```bash
./br [script_args] -- [python_args]
```

Script arguments:
- `--force`: Force regeneration of test files even if they already exist.

Python arguments (passed after `--`):
- Any arguments to be passed to `generate_test_files.py`.

Examples:
- `./br`: Build both implementations and generate test files if they don't exist.
- `./br --force`: Build and force regeneration of test files.
- `./br -- --size 1000000 --complexity high`: Build and generate specific test files.
- `./br --force -- --size 1000000 --complexity high`: Build, force regenerate specific test files.

## Usage

### Comparing Files

For the C++ version:
```bash
./cmp <file1> <file2>
```

For the Rust version:
```bash
./rust_compare/target/release/rust_compare <file1> <file2>
```

Where:
- `file1`: Path to the first file.
- `file2`: Path to the second file.

### Generating Test Files

Use the Python script directly or through the `br` script to generate large test files:
```bash
python3 generate_test_files.py [options]
```

Options:
- `--size`: Total number of lines in each file (default: 1,000,000).
- `--line_length`: Length of each line (default: 100 characters).
- `--diff_percentage`: Percentage of lines that should differ between the files.
- `--chars_to_change`: Number of characters to change in each different line.

Example:
```bash
python3 generate_test_files.py --size 500000 --line_length 80 --diff_percentage 5 --chars_to_change 10
```

This generates `test_file_1.txt` and `test_file_2.txt` with the specified differences.

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

The `br` script automatically runs performance comparisons if both builds are successful. Alternatively, use the `compare_performance.sh` script directly:
```bash
./compare_performance.sh test_file_1.txt test_file_2.txt
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
