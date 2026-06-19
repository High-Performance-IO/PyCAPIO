#ifndef PYCAPIO_LIBCAPIO_HPP
#define PYCAPIO_LIBCAPIO_HPP
#include <dirent.h>
#include <filesystem>

#include "common/constants.hpp"

class StartupSemaphore final {
    int fp;
    const std::string lock_to_check;

  public:
    explicit StartupSemaphore(const std::string &workflow_name);
    ~StartupSemaphore();
    explicit operator bool() const;
};

int bootstrap_capio_server(const std::filesystem::path &CAPIO_DIR,
                           const std::string &CAPIO_WORKFLOW_NAME,
                           const std::string &capio_server_exec_path,
                           const std::string &capio_cl_config,
                           const int await_server_timeout_seconds);

int libcapio_init(const std::filesystem::path &CAPIO_DIR    = ".",
                  const std::string &CAPIO_APP_NAME         = CAPIO_DEFAULT_APP_NAME,
                  const std::string &CAPIO_WORKFLOW_NAME    = CAPIO_DEFAULT_WORKFLOW_NAME,
                  const std::string &capio_server_exec_path = "capio_server",
                  const std::string &capio_cl_config        = "",
                  const int await_server_timeout_seconds    = 2);

void libcapio_teardown(const bool teardown_server = false);

int libcapio_open(const char *path, int flags, mode_t mode = 0);
long libcapio_read(const int fd, char *buf, const size_t size);
long libcapio_write(const int fd, const char *buf, const size_t size);
long libcapio_close(const int fd);
long libcapio_readdir(const int fd, dirent64 *entry);
long libcapio_mkdir(const char *path, int mode);
long libcapio_lseek(int fd, long offset, int whence);
long libcapio_stat(const char *path, struct stat *statbuf);

#endif // PYCAPIO_LIBCAPIO_HPP
