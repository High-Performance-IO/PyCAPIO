#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "libcapio.hpp"

#include "include/_CapioIOWrapper.hpp"

static const char *path  = "/tmp/sample.dat";
static const char *path1 = "/tmp/sample1.dat";

static const std::string first_line  = "first line\n";
static const std::string second_line = "second line\n";
static const std::string third_line  = "third line\n";

void test_write() {
    std::cout << "TEST: " << __func__ << "..." << std::flush;
    int fd = libcapio_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);

    _CapioIOWrapper f(fd);

    f.write(first_line);
    f.write(second_line);
    f.write(third_line);

    libcapio_close(fd);
    std::cout << "PASSED" << std::endl;
}

void test_read_with_size() {
    std::cout << "TEST: " << __func__ << "..." << std::flush;
    int fd = libcapio_open(path, O_RDONLY);
    assert(fd >= 0);

    _CapioIOWrapper f(fd);

    std::string content;

    content = f.read(11);
    assert(content == first_line);

    content = f.read(12);
    assert(content == second_line);

    content = f.read(11);
    assert(content == third_line);

    libcapio_close(fd);
    std::cout << "PASSED" << std::endl;
}

void test_read() {
    std::cout << "TEST: " << __func__ << "..." << std::flush;
    int fd = libcapio_open(path, O_RDONLY);
    assert(fd >= 0);

    _CapioIOWrapper f(fd);
    std::string content = f.read();

    assert(content == first_line + second_line + third_line);

    libcapio_close(fd);
    std::cout << "PASSED" << std::endl;
}

void test_readlines() {
    std::cout << "TEST: " << __func__ << "..." << std::flush;
    int fd = libcapio_open(path, O_RDONLY);
    assert(fd >= 0);

    _CapioIOWrapper f(fd);
    std::vector<std::string> lines;

    while (true) {
        std::string line = f.readline();
        if (line.empty()) {
            break;
        }
        lines.push_back(line);
    }

    const std::vector expected = {first_line, second_line, third_line};

    assert(lines == expected);

    libcapio_close(fd);
    std::cout << "PASSED" << std::endl;
}

void test_write_1gb() {
    std::cout << "TEST: " << __func__ << "..." << std::flush;
    int fd = libcapio_open(path1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd >= 0);

    _CapioIOWrapper f(fd);

    constexpr size_t chunk = 1024;
    std::string buf(chunk, '1');

    for (size_t i = 0; i < 1024 * 1024; ++i) {
        f.write(buf);
    }

    libcapio_close(fd);
    std::cout << "PASSED" << std::endl;
}

void test_read_1gb() {
    std::cout << "TEST: " << __func__ << "..." << std::flush;
    int fd = libcapio_open(path1, O_RDONLY);
    assert(fd >= 0);

    _CapioIOWrapper f(fd);
    std::string data = f.read();

    if (data.size() != 1024 * 1024 * 1024) {
        std::cout << std::endl
                  << "Error: read size is " << data.size() << " instead of " << 1024 * 1024 * 1024
                  << std::endl;
        assert(data.size() == 1024 * 1024 * 1024);
    }

    for (char c : data) {
        assert(c == '1');
    }

    libcapio_close(fd);
    std::cout << "PASSED" << std::endl;
}

int main() {
    // Environment setup (equivalent to pytest)
    setenv("CAPIO_DIR", "/tmp", 1);
    setenv("DEBUG", "on", 1);
    setenv("CAPIO_LOG_LEVEL", "-1", 1);

    libcapio_init();

    test_write();
    test_read_with_size();
    test_read();
    test_readlines();
    test_write_1gb();
    test_read_1gb();

    libcapio_teardown();

    std::cout << "All tests passed.\n";
    return 0;
}
