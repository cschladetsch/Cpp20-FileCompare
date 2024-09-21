#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <span>
#include <filesystem>
#include <iomanip>

std::vector<std::string> read_file(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(std::move(line));
    }
    return lines;
}

std::string create_diff_marker(const std::string& s1, const std::string& s2) {
    std::string marker(std::max(s1.length(), s2.length()), ' ');
    for (size_t i = 0; i < std::min(s1.length(), s2.length()); ++i) {
        if (s1[i] != s2[i]) {
            marker[i] = '^';
        }
    }
    if (s1.length() != s2.length()) {
        std::fill(marker.begin() + std::min(s1.length(), s2.length()), marker.end(), '^');
    }
    return marker;
}

void print_diff_line(std::ostream& out, char prefix, size_t line_num, const std::string& line, size_t max_line_num) {
    out << prefix << ' ' << std::setw(std::to_string(max_line_num).length()) << line_num << ": " << line << '\n';
}

void print_marker_line(std::ostream& out, const std::string& marker, size_t max_line_num) {
    out << std::string(std::to_string(max_line_num).length() + 3, ' ') << marker << '\n';
}

void compare_files(std::span<const std::string> file1_lines, 
                   std::span<const std::string> file2_lines,
                   std::atomic<size_t>& diff_count,
                   std::mutex& output_mutex) {
    const size_t chunk_size = 1000;
    const size_t num_chunks = (file1_lines.size() + chunk_size - 1) / chunk_size;
    const size_t max_line_num = std::max(file1_lines.size(), file2_lines.size());

    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_chunks; ++i) {
        threads.emplace_back([&, i]() {
            const size_t start = i * chunk_size;
            const size_t end = std::min(start + chunk_size, file1_lines.size());

            std::ostringstream local_output;

            for (size_t j = start; j < end; ++j) {
                if (j >= file2_lines.size() || file1_lines[j] != file2_lines[j]) {
                    diff_count.fetch_add(1, std::memory_order_relaxed);
                    if (j >= file2_lines.size()) {
                        print_diff_line(local_output, '+', j + 1, file1_lines[j], max_line_num);
                        print_marker_line(local_output, std::string(file1_lines[j].length(), '^'), max_line_num);
                    } else if (j >= file1_lines.size()) {
                        print_diff_line(local_output, '-', j + 1, file2_lines[j], max_line_num);
                        print_marker_line(local_output, std::string(file2_lines[j].length(), '^'), max_line_num);
                    } else {
                        print_diff_line(local_output, '-', j + 1, file2_lines[j], max_line_num);
                        print_diff_line(local_output, '+', j + 1, file1_lines[j], max_line_num);
                        print_marker_line(local_output, create_diff_marker(file2_lines[j], file1_lines[j]), max_line_num);
                    }
                }
            }

            std::scoped_lock lock(output_mutex);
            std::cout << local_output.str();
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

    auto file1_lines = read_file(file1_name);
    auto file2_lines = read_file(file2_name);

    std::atomic<size_t> diff_count = 0;
    std::mutex output_mutex;

    compare_files(file1_lines, file2_lines, diff_count, output_mutex);

    std::cout << "Total differences: " << diff_count << '\n';

    return 0;
}
