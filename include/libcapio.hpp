#ifndef PYCAPIO_LIBCAPIO_HPP
#define PYCAPIO_LIBCAPIO_HPP
#include <dirent.h>
#include <filesystem>

#include "common/constants.hpp"

/**
 * @brief RAII guard that serialises CAPIO server startup across processes.
 *
 * On construction it acquires a per-workflow lock so that only one process
 * bootstraps the CAPIO server for a given workflow; the lock is released on
 * destruction. Evaluating the object in a boolean context reports whether this
 * process is the one responsible for starting the server.
 */
class StartupSemaphore final {
    int fp;                            ///< File descriptor backing the lock.
    const std::string lock_to_check;  ///< Path of the workflow lock file.

  public:
    /// @brief Acquire the startup lock for @p workflow_name.
    /// @param workflow_name Workflow whose startup is being guarded.
    explicit StartupSemaphore(const std::string &workflow_name);

    /// @brief Release the startup lock.
    ~StartupSemaphore();

    /// @brief Report whether this process should start the CAPIO server.
    /// @return @c true if this process holds the right to bootstrap the server.
    explicit operator bool() const;
};

/**
 * @brief Start the CAPIO server process for a workflow and wait until it is ready.
 * @param CAPIO_DIR Root directory managed by CAPIO.
 * @param CAPIO_WORKFLOW_NAME Logical workflow name reported to CAPIO.
 * @param capio_server_exec_path Path or name of the CAPIO server executable.
 * @param capio_cl_config Optional path to a CAPIO-CL configuration file.
 * @param await_server_timeout_seconds Seconds to wait for the server to be ready.
 * @return Status code (0 on success).
 */
int bootstrap_capio_server(const std::filesystem::path &CAPIO_DIR,
                           const std::string &CAPIO_WORKFLOW_NAME,
                           const std::string &capio_server_exec_path,
                           const std::string &capio_cl_config, int await_server_timeout_seconds);

/**
 * @brief Initialise CAPIO and start or attach to the CAPIO server.
 * @param CAPIO_DIR Root directory managed by CAPIO.
 * @param CAPIO_APP_NAME Logical application name reported to CAPIO.
 * @param CAPIO_WORKFLOW_NAME Logical workflow name reported to CAPIO.
 * @param capio_server_exec_path Path or name of the CAPIO server executable.
 * @param capio_cl_config Optional path to a CAPIO-CL configuration file.
 * @param await_server_timeout_seconds Seconds to wait for the server to be ready.
 * @return Status code (0 on success).
 */
int libcapio_init(const std::filesystem::path &CAPIO_DIR    = ".",
                  const std::string &CAPIO_APP_NAME         = CAPIO_DEFAULT_APP_NAME,
                  const std::string &CAPIO_WORKFLOW_NAME    = CAPIO_DEFAULT_WORKFLOW_NAME,
                  const std::string &capio_server_exec_path = "capio_server",
                  const std::string &capio_cl_config = "", int await_server_timeout_seconds = 2);

/**
 * @brief Tear down CAPIO, optionally stopping the server process.
 * @param teardown_server When @c true, also shut down the CAPIO server.
 */
void libcapio_teardown(bool teardown_server = false);

/**
 * @brief Open a CAPIO-managed file.
 * @param path File path inside the CAPIO directory.
 * @param flags POSIX open flags (see @c FILE_MODES on the Python side).
 * @param mode Permission bits applied when the file is created.
 * @return A file descriptor on success.
 */
int libcapio_open(const char *path, int flags, mode_t mode = 0);

/**
 * @brief Read up to @p size bytes from a CAPIO file descriptor.
 * @param fd Open CAPIO file descriptor.
 * @param buf Destination buffer of at least @p size bytes.
 * @param size Maximum number of bytes to read.
 * @return The number of bytes read, @c 0 at EOF, or a negative value on error.
 */
long libcapio_read(int fd, char *buf, size_t size);

/**
 * @brief Write @p size bytes to a CAPIO file descriptor.
 * @param fd Open CAPIO file descriptor.
 * @param buf Source buffer holding the data to write.
 * @param size Number of bytes to write.
 * @return The number of bytes written, or a negative value on error.
 */
long libcapio_write(int fd, const char *buf, size_t size);

/**
 * @brief Close a CAPIO file descriptor.
 * @param fd File descriptor to close.
 * @return @c 0 on success or a negative value on error.
 */
long libcapio_close(int fd);

/**
 * @brief Read the next directory entry from a CAPIO directory descriptor.
 * @param fd Open CAPIO directory descriptor.
 * @param entry Output parameter that receives the next entry.
 * @return A positive value when an entry was read, @c 0 at the end of the
 *         directory, or a negative value on error.
 */
long libcapio_readdir(int fd, dirent64 *entry);

/**
 * @brief Create a directory inside the CAPIO directory.
 * @param path Directory path to create.
 * @param mode Permission bits for the new directory.
 */
long libcapio_mkdir(const char *path, int mode);

/**
 * @brief Reposition the offset of a CAPIO file descriptor.
 * @param fd Open CAPIO file descriptor.
 * @param offset Byte offset relative to @p whence.
 * @param whence Reference point (@c SEEK_SET, @c SEEK_CUR or @c SEEK_END).
 * @return The resulting absolute offset, or a negative value on error.
 */
long libcapio_lseek(int fd, long offset, int whence);

/**
 * @brief Retrieve metadata for a CAPIO-managed path.
 * @param path Path to stat.
 * @param statbuf Output parameter that receives the file metadata.
 * @return @c 0 on success or a negative value on error.
 */
long libcapio_stat(const char *path, struct stat *statbuf);

#endif // PYCAPIO_LIBCAPIO_HPP
