#include "../libcapio.hpp"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

void print_usage(const char *prog_name) {
    std::cout << "Usage:\n"
              << "  " << prog_name << " --write <size_in_bytes>\n"
              << "  " << prog_name << " --read <size_to_read>\n";
}

int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string mode{argv[1]};
    const size_t target_size{static_cast<size_t>(std::stoll(argv[2]))};
    constexpr char file_path[]{"/tmp/test.dat"};

    libcapio_init("/tmp");

    if (mode == "--write") {
        const std::vector<char> buffer(target_size, 'A');

        const int f{libcapio_open(file_path, O_RDWR | O_CREAT, 0644)};
        if (f >= 0) {
            const ssize_t written = libcapio_write(f, buffer.data(), buffer.size());
            std::cout << "Successfully wrote " << written << " bytes.\n";
            libcapio_close(f);
        } else {
            std::cerr << "Error opening file for writing: strerror:" << strerror(errno) << "\n";
        }

    } else if (mode == "--read") {
        std::vector<char> recv_buffer(target_size);

        const int f{libcapio_open(file_path, O_RDONLY)};
        if (f >= 0) {
            if (ssize_t bytes_read = libcapio_read(f, recv_buffer.data(), recv_buffer.size());
                bytes_read > 0) {
                std::cout << "Read " << bytes_read << " bytes from file.\n";
            }
            libcapio_close(f);
        } else {
            std::cerr << "Error opening file for reading: fd=" << f << "\n";
        }
    }

    libcapio_teardown();
    return 0;
}