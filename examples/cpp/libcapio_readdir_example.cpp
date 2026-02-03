#include "../../libcapio.hpp"

#include <thread>

void create_function() {
    libcapio_init("/tmp", "producer", "libcapio_readdir_example");
    libcapio_close(libcapio_open("/tmp/test1.txt", O_CREAT | O_RDWR, O_APPEND));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    libcapio_close(libcapio_open("/tmp/test2.txt", O_CREAT | O_RDWR, O_APPEND));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    libcapio_close(libcapio_open("/tmp/test3.txt", O_CREAT | O_RDWR, O_APPEND));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    libcapio_close(libcapio_open("/tmp/test4.txt", O_CREAT | O_RDWR, O_APPEND));
    libcapio_teardown();
}

void list_function() {
    libcapio_init("/tmp", "consumer", "libcapio_readdir_example");
    int dirfd = libcapio_open("/tmp", 0, O_RDONLY);

    dirent64 dir{};

    while (libcapio_readdir(dirfd, &dir) != NULL) {
        std::cout << dir.d_name << std::endl;
    }

    libcapio_close(dirfd);

    libcapio_teardown();
}

int main() {

    std::thread t0(list_function);
    std::thread t1(create_function);

    t1.join();
    t0.join();
    return 0;
}