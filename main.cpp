#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <span>
#include <filesystem>
#include <iomanip>
#include <cstring>
#include <memory>

constexpr size_t MAX_LINE_LENGTH = 1024;
constexpr size_t MAX_LINES = 1000000;
constexpr size_t CHUNK_SIZE = 1000;

struct LineInfo {
    char* data;
    size_t length;
};

class FileLines {
private:
    std::unique_ptr<char[]> buffer;
    std::vector<LineInfo> lines;

public:
    FileLines(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        size_t file_size = file.tellg();
        file.seekg(0);

        buffer = std::make_unique<char[]>(file_size + 1);
        file.read(buffer.get(), file_size);
        buffer[file_size] = '\0';

        char* start = buffer.get();
        char* end = start;
        while (*end) {
            if (*end == '\n') {
                lines.push_back({start, static_cast<size_t>(end - start)});
                start = end + 1;
            }
            ++end;
        }
        if (start != end) {
            lines.push_back({start, static_cast<size_t>(end - start)});
        }
    }

    std::span<const LineInfo> get_lines() const {
        return lines;
    }
};

inline bool compare_lines(const LineInfo& l1, const LineInfo& l2) {
    return l1.length == l2.length && std::memcmp(l1.data, l2.data, l1.length) == 0;
}

void print_diff_line(std::ostream& out, char prefix, size_t line_num, const LineInfo& line, size_t max_line_num) {
    out << prefix << ' ' << std::setw(std::to_string(max_line_num).length()) << line_num << ": " 
        << std::string_view(line.data, line.length) << '\n';
}

void print_marker_line(std::ostream& out, const std::string& marker, size_t max_line_num) {
    out << std::string(std::to_string(max_line_num).length() + 3, ' ') << marker << '\n';
}

std::string create_diff_marker(const LineInfo& s1, const LineInfo& s2) {
    std::string marker(std::max(s1.length, s2.length), ' ');
    for (size_t i = 0; i < std::min(s1.length, s2.length); ++i) {
        if (s1.data[i] != s2.data[i]) {
            marker[i] = '^';
        }
    }
    if (s1.length != s2.length) {
        std::fill(marker.begin() + std::min(s1.length, s2.length), marker.end(), '^');
    }
    return marker;
}

void compare_files(std::span<const LineInfo> file1_lines, 
                   std::span<const LineInfo> file2_lines,
                   std::atomic<size_t>& diff_count,
                   std::mutex& output_mutex) {
    const size_t num_chunks = (file1_lines.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;
    const size_t max_line_num = std::max(file1_lines.size(), file2_lines.size());

    std::vector<std::thread> threads;
    threads.reserve(num_chunks);

    for (size_t i = 0; i < num_chunks; ++i) {
        threads.emplace_back([&, i]() {
            const size_t start = i * CHUNK_SIZE;
            const size_t end = std::min(start + CHUNK_SIZE, file1_lines.size());

            std::string local_output;
            local_output.reserve(CHUNK_SIZE * MAX_LINE_LENGTH);

            for (size_t j = start; j < end; ++j) {
                if (j >= file2_lines.size() || !compare_lines(file1_lines[j], file2_lines[j])) {
                    diff_count.fetch_add(1, std::memory_order_relaxed);
                    if (j >= file2_lines.size()) {
                        print_diff_line(local_output, '+', j + 1, file1_lines[j], max_line_num);
                        print_marker_line(local_output, std::string(file1_lines[j].length, '^'), max_line_num);
                    } else if (j >= file1_lines.size()) {
                        print_diff_line(local_output, '-', j + 1, file2_lines[j], max_line_num);
                        print_marker_line(local_output, std::string(file2_lines[j].length, '^'), max_line_num);
                    } else {
                        print_diff_line(local_output, '-', j + 1, file2_lines[j], max_line_num);
                        print_diff_line(local_output, '+', j + 1, file1_lines[j], max_line_num);
                        print_marker_line(local_output, create_diff_marker(file2_lines[j], file1_lines[j]), max_line_num);
                    }
                }
            }

            std::scoped_lock lock(output_mutex);
            std::cout << local_output;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file1> <file2>\n";
        return 1;
    }

    const std::string file1_name = argv[1];
    const std::string file2_name = argv[2];

    if (!std::filesystem::exists(file1_name) || !std::filesystem::exists(file2_name)) {
        std::cerr << "Error: One or both files do not exist.\n";
        return 1;
    }

    try {
        FileLines file1_lines(file1_name);
        FileLines file2_lines(file2_name);

        std::atomic<size_t> diff_count = 0;
        std::mutex output_mutex;

        compare_files(file1_lines.get_lines(), file2_lines.get_lines(), diff_count, output_mutex);

        std::cout << "Total differences: " << diff_count << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
