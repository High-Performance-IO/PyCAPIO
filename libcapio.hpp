#ifndef PYCAPIO_PYCAPIO_HPP
#define PYCAPIO_PYCAPIO_HPP

/**
 * Capio log level.
 * if -1, and capio logging is enable everything is logged, otherwise, only
 * logs up to CAPIO_MAX_LOG_LEVEL function calls
 */
inline bool syscall_no_intercept_flag;
#define syscall_no_intercept syscall
#define CAPIO_VERSION ""

#include <array>
#include <string>

#include "common/syscall.hpp"

#include "utils/clone.hpp"
#include "utils/filesystem.hpp"
#include "utils/snapshot.hpp"

#include "handlers.hpp"

inline void libcapio_init(const std::filesystem::path &CAPIO_DIR = ".",
                          const std::string &CAPIO_APP_NAME      = CAPIO_DEFAULT_APP_NAME,
                          const std::string &CAPIO_WORKFLOW_NAME = CAPIO_DEFAULT_WORKFLOW_NAME) {

    if (getenv("CAPIO_APP_NAME") == nullptr) {
        setenv("CAPIO_APP_NAME", CAPIO_APP_NAME.c_str(), 1);
    }

    if (getenv("CAPIO_DIR") == nullptr) {
        setenv("CAPIO_DIR", CAPIO_DIR.string().c_str(), 1);
    }

    if (std::getenv("CAPIO_WORKFLOW_NAME") == nullptr) {
        setenv("CAPIO_WORKFLOW_NAME", CAPIO_WORKFLOW_NAME.c_str(), 1);
    }

    std::cout << "LIBCAPIO | CAPIO_DIR: " << getenv("CAPIO_DIR") << std::endl;
    std::cout << "LIBCAPIO | CAPIO_APP_NAME: " << getenv("CAPIO_APP_NAME") << std::endl;
    std::cout << "LIBCAPIO | CAPIO_WORKFLOW_NAME: " << getenv("CAPIO_WORKFLOW_NAME") << std::endl;

    init_client(gettid());
    init_filesystem();
    initialize_new_thread();

    START_SYSCALL_LOGGING();
}

inline void libcapio_teardown() { exit_handler(NULL, NULL, NULL, NULL, NULL, NULL, NULL); }

inline auto libcapio_open(const char *path, int flags, mode_t mode = 0) {
    START_LOG(gettid(), "call(path=%s)", path);
    long result;
    openat_handler(AT_FDCWD, reinterpret_cast<long>(path), flags, NULL, NULL, NULL, &result);
    LOG("open_handler result = %d", result);

    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        LOG("Delegating to glibc");
        return open(path, flags, mode);
    }

    if (result == CAPIO_POSIX_SYSCALL_ERRNO) {
        LOG("Error has occurred while opening file from server side! errno=%s", strerror(errno));
    }

    return static_cast<int>(result);
}

inline auto libcapio_read(const int fd, char *buf, const size_t size) {
    long result;
    read_handler(fd, reinterpret_cast<long>(buf), size, NULL, NULL, NULL, &result);
    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        return read(fd, buf, size);
    }
    return result;
}

inline auto libcapio_write(const int fd, const char *buf, const size_t size) {
    long result;
    write_handler(fd, reinterpret_cast<long>(buf), size, NULL, NULL, NULL, &result);
    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        return write(fd, buf, size);
    }
    return result;
}

inline auto libcapio_close(const int fd) {
    long result;
    close_handler(fd, NULL, NULL, NULL, NULL, NULL, &result);
    return result;
}

#endif // PYCAPIO_PYCAPIO_HPP
