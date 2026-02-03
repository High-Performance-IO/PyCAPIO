#include "../../libcapio.hpp"

int main() {

    libcapio_init("/tmp");
    int fd1 = libcapio_open("/tmp/test1.txt", O_CREAT | O_RDWR, O_APPEND);
    int fd2 = libcapio_open("/tmp/test2.txt", O_CREAT | O_RDWR, O_APPEND);
    int fd3 = libcapio_open("/tmp/test3.txt", O_CREAT | O_RDWR, O_APPEND);
    int fd4 = libcapio_open("/tmp/test4.txt", O_CREAT | O_RDWR, O_APPEND);
    int dirfd = libcapio_open("/tmp", 0, O_RDONLY);

    dirent64 dir{};

    while (libcapio_readdir(dirfd, &dir) != NULL) {
        std::cout << dir.d_name << std::endl;
    }

    libcapio_close(fd1);
    libcapio_close(fd2);
    libcapio_close(fd3);
    libcapio_close(fd4);
    libcapio_close(dirfd);

    libcapio_teardown();
    return 0;
}