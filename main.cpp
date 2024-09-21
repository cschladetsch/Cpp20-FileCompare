#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

constexpr size_t CHUNK_SIZE = 1 << 20; // 1MB chunks
constexpr size_t MAX_THREADS = 64;

struct MappedFile {
    int fd;
    char* data;
    size_t size;

    MappedFile(const char* filename) {
        fd = open(filename, O_RDONLY);
        if (fd == -1) throw std::runtime_error("Failed to open file");

        struct stat sb;
        if (fstat(fd, &sb) == -1) throw std::runtime_error("Failed to get file size");
        size = sb.st_size;

        data = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (data == MAP_FAILED) throw std::runtime_error("Failed to map file");
    }

    ~MappedFile() {
        munmap(data, size);
        close(fd);
    }
};

struct DiffInfo {
    size_t line;
    size_t pos;
    bool is_diff;
};

void find_diffs(const char* data1, const char* data2, size_t start, size_t end, 
                std::vector<DiffInfo>& diffs, std::atomic<size_t>& total_diffs) {
    size_t line = 1;
    size_t pos = start;
    while (pos < end) {
        if (data1[pos] != data2[pos]) {
            diffs.push_back({line, pos, true});
            total_diffs.fetch_add(1, std::memory_order_relaxed);
            while (pos < end && data1[pos] != '\n' && data2[pos] != '\n') pos++;
        } else if (data1[pos] == '\n') {
            line++;
        }
        pos++;
    }
}

void print_diffs(const char* data1, const char* data2, const std::vector<DiffInfo>& diffs) {
    for (const auto& diff : diffs) {
        std::cout << "Diff at line " << diff.line << ", position " << diff.pos << ":\n";
        size_t start = diff.pos;
        while (start > 0 && data1[start - 1] != '\n') start--;
        size_t end = diff.pos;
        while (data1[end] != '\n' && data1[end] != '\0') end++;
        std::cout << "File 1: " << std::string_view(data1 + start, end - start) << "\n";
        std::cout << "File 2: " << std::string_view(data2 + start, end - start) << "\n\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <file1> <file2>\n";
        return 1;
    }

    try {
        MappedFile file1(argv[1]);
        MappedFile file2(argv[2]);

        if (file1.size != file2.size) {
            std::cout << "Files have different sizes. They are not identical.\n";
            return 0;
        }

        std::atomic<size_t> total_diffs = 0;
        std::vector<std::thread> threads;
        std::vector<std::vector<DiffInfo>> thread_diffs(MAX_THREADS);

        size_t chunk_size = std::max(CHUNK_SIZE, file1.size / MAX_THREADS);
        size_t num_chunks = (file1.size + chunk_size - 1) / chunk_size;

        for (size_t i = 0; i < num_chunks; ++i) {
            size_t start = i * chunk_size;
            size_t end = std::min(start + chunk_size, file1.size);
            threads.emplace_back(find_diffs, file1.data, file2.data, start, end, 
                                 std::ref(thread_diffs[i % MAX_THREADS]), std::ref(total_diffs));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        if (total_diffs == 0) {
            std::cout << "Files are identical.\n";
        } else {
            std::cout << "Total differences: " << total_diffs << "\n\n";
            for (const auto& diffs : thread_diffs) {
                print_diffs(file1.data, file2.data, diffs);
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
