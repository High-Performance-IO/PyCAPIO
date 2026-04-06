#ifndef PYCAPIO_PYCAPIO_HPP
#define PYCAPIO_PYCAPIO_HPP

#ifndef __LIBCAPIO
#define __LIBCAPIO
#include <unistd.h>
#include <thread>
#endif

/**
 * Capio log level.
 * if -1, and capio logging is enable everything is logged, otherwise, only
 * logs up to CAPIO_MAX_LOG_LEVEL function calls
 */
inline bool syscall_no_intercept_flag;
#define syscall_no_intercept syscall
#define CAPIO_VERSION ""

#include <dirent.h>
#include <string>

#include "common/syscall.hpp"

#include "utils/clone.hpp"
#include "utils/filesystem.hpp"
#include "utils/snapshot.hpp"

#include "common/logger.hpp"

#include "handlers.hpp"

static thread_local std::string libcapio_preamble;

class StartupSemaphore final {
    int fp;
    const std::string lock_to_check;

  public:
    explicit StartupSemaphore(const std::string &workflow_name)
        : lock_to_check(workflow_name + ".lock") {
        fp = shm_open(lock_to_check.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    }

    ~StartupSemaphore() {
        if (fp != -1) {
            close(fp);
            unlink(lock_to_check.c_str());
        }
    }

    explicit operator bool() const { return fp != -1; }
};

static bool libcapio_initialized = false;

inline void bootstrap_capio_server(const std::filesystem::path &CAPIO_DIR,
                                   const std::string &CAPIO_WORKFLOW_NAME,
                                   const std::string &capio_server_exec_path,
                                   const std::string &capio_cl_config,
                                   const int await_server_timeout_seconds) {
    bool server_is_started = false;

    std::string shm_name = "/dev/shm/" + CAPIO_WORKFLOW_NAME;
    if(std::filesystem::exists(shm_name)){
        std::cout << libcapio_preamble << " CAPIO server is already started" << std::endl;
        server_is_started = true;
    }

    // check if server instance exists. If not, boot a server instance
    if (!server_is_started) {
        const StartupSemaphore boot_lock(CAPIO_WORKFLOW_NAME);
        if (boot_lock) {
            static constexpr char NO_CONFIG_FLAG[]   = "--no-config";
            static constexpr char CONFIG_FLAG[]      = "--config";
            static constexpr char BACKEND_FLAG[]     = "--backend";
            static constexpr char NO_BACKEND_AFTER[] = "none";

            std::cout << libcapio_preamble << " Booting up CAPIO server instance" << std::endl;

            std::vector<std::string> newEnv;
            for (char **env = environ; *env != nullptr; ++env) {
                newEnv.emplace_back(*env);
            }

            newEnv.emplace_back("CAPIO_DIR=" + CAPIO_DIR.string());
            newEnv.emplace_back("CAPIO_LOG_LEVEL=-1");
            newEnv.emplace_back("CAPIO_WORKFLOW_NAME=" + CAPIO_WORKFLOW_NAME);

            std::vector<char *> envPtrs;
            for (auto &s : newEnv) {
                envPtrs.push_back(&s[0]);
            }
            envPtrs.push_back(nullptr);

            // resolve capio_server executable if not provided as absolute path to a binary from
            // PATH
            std::string resolved_capio_server_exec_path = capio_server_exec_path;
            if (capio_server_exec_path == "capio_server") {
                const auto path = std::getenv("PATH");
                std::stringstream ss(path);
                std::string item;

                while (std::getline(ss, item, ':')) {
                    std::filesystem::path check(item);
                    check /= capio_server_exec_path;
                    if (std::filesystem::exists(check)) {
                        resolved_capio_server_exec_path = check;
                        break;
                    }
                }
            }

            // Fail if capio_server binary executable does not exists
            if (!std::filesystem::exists(resolved_capio_server_exec_path)) {
                std::cerr << libcapio_preamble
                          << " Could not locate capio_server executable in PATH!" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            char *args[6] = {const_cast<char *>(resolved_capio_server_exec_path.c_str()),
                             capio_cl_config.empty() ? const_cast<char *>(NO_CONFIG_FLAG)
                                                     : const_cast<char *>(CONFIG_FLAG),
                             capio_cl_config.empty() ? nullptr
                                                     : const_cast<char *>(capio_cl_config.c_str()),
                             const_cast<char *>(BACKEND_FLAG),
                             const_cast<char *>(NO_BACKEND_AFTER),
                             nullptr};

            if (const pid_t pid = fork(); pid == 0) {
                execve(args[0], args, envPtrs.data());
                std::cerr << libcapio_preamble << " " + capio_server_exec_path << std::endl;
                std::cerr << libcapio_preamble << " EXECVE failure: " + std::string(strerror(errno))
                          << std::endl;
                std::exit(EXIT_FAILURE);
            } else if (pid > 0) {
                std::cout << libcapio_preamble << " Started CAPIO server with  PID: " << pid
                          << std::endl;
                sleep(await_server_timeout_seconds);
                // jump to attaching to shm queues and await normal execution
            } else {
                std::cerr << libcapio_preamble
                          << "Failed to FORK server thread. Error: " +
                                 std::string(std::strerror(errno))
                          << std::endl;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        }
    }
}

inline void libcapio_init(const std::filesystem::path &CAPIO_DIR    = ".",
                          const std::string &CAPIO_APP_NAME         = CAPIO_DEFAULT_APP_NAME,
                          const std::string &CAPIO_WORKFLOW_NAME    = CAPIO_DEFAULT_WORKFLOW_NAME,
                          const std::string &capio_server_exec_path = "capio_server",
                          const std::string &capio_cl_config        = "",
                          const int await_server_timeout_seconds    = 2) {

    if (libcapio_preamble.empty()) {
        char host_name[HOST_NAME_MAX]{0};
        if(gethostname(host_name, HOST_NAME_MAX) == -1){
		    std::cout << "WARN: gethostname failed: " << std::strerror(errno) << std::endl;
	    }
	const auto pid    = getpid();
        libcapio_preamble = "[[LIBCAPIO::" + std::string(host_name) + "::" + CAPIO_APP_NAME +
                            "::" + std::to_string(pid) + "]] ";
    }

    START_LOG(gettid(), "libcapio_init(CAPIO_DIR=%s, CAPIO_APP_NAME=%s, CAPIO_WORKFLOW_NAME=%s)",
              CAPIO_DIR.c_str(), CAPIO_APP_NAME.c_str(), CAPIO_WORKFLOW_NAME.c_str());

#ifdef CAPIO_LOG
    std::cout << libcapio_preamble << " WARNING: LIBCAPIO HAS BEEN COMPILED IN RELEASE MODE!"
              << std::endl;
#endif

    if (getenv("CAPIO_APP_NAME") == nullptr) {
        setenv("CAPIO_APP_NAME", CAPIO_APP_NAME.c_str(), 1);
    }

    if (getenv("CAPIO_DIR") == nullptr) {
        setenv("CAPIO_DIR", CAPIO_DIR.string().c_str(), 1);
    }

    if (std::getenv("CAPIO_WORKFLOW_NAME") == nullptr) {
        setenv("CAPIO_WORKFLOW_NAME", CAPIO_WORKFLOW_NAME.c_str(), 1);
    }

    if (libcapio_initialized) {
        return;
    }

    bootstrap_capio_server(CAPIO_DIR, CAPIO_WORKFLOW_NAME, capio_server_exec_path, capio_cl_config,
                           await_server_timeout_seconds);

    init_client(gettid());
    init_filesystem();
    initialize_new_thread();

    libcapio_initialized = true;

    START_SYSCALL_LOGGING();

    if (const auto silent = std::getenv("SILENT");
        silent != nullptr && std::string(silent) == "OFF") {
        std::cout << libcapio_preamble << " CAPIO_DIR: " << getenv("CAPIO_DIR") << std::endl;
        std::cout << libcapio_preamble << " CAPIO_APP_NAME: " << getenv("CAPIO_APP_NAME")
                  << std::endl;
        std::cout << libcapio_preamble << " CAPIO_WORKFLOW_NAME: " << getenv("CAPIO_WORKFLOW_NAME")
                  << std::endl;
        std::cout << libcapio_preamble << " PID - TID: " << std::to_string(getpid()) << " - "
                  << std::to_string(gettid()) << std::endl
                  << std::endl;
    }

    LOG("\n\n");
}

inline void libcapio_teardown() {
    START_LOG(gettid(), "libcapio_teardown()");
    if (libcapio_initialized) {
        exit_handler(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        libcapio_initialized = false;
    }

    LOG("\n\n");
}

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

    LOG("\n\n");
    return static_cast<int>(result);
}

inline auto libcapio_read(const int fd, char *buf, const size_t size) {
    START_LOG(gettid(), "call(fd=%d, size=%ld)", fd, size);
    long result;
    read_handler(fd, reinterpret_cast<long>(buf), size, NULL, NULL, NULL, &result);
    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        return read(fd, buf, size);
    }

    LOG("\n\n");
    return result;
}

inline auto libcapio_write(const int fd, const char *buf, const size_t size) {
    START_LOG(gettid(), "call(fd=%d, size=%ld)", fd, size);
    long result;
    write_handler(fd, reinterpret_cast<long>(buf), size, NULL, NULL, NULL, &result);
    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        return write(fd, buf, size);
    }

    LOG("\n\n");
    return result;
}

inline auto libcapio_close(const int fd) {
    START_LOG(gettid(), "call(fd=%ld)", fd);
    long result;
    close_handler(fd, NULL, NULL, NULL, NULL, NULL, &result);

    LOG("\n\n");
    return result;
}

inline auto libcapio_readdir(const int fd, dirent64 *entry) {
    START_LOG(gettid(), "call(fd=%d)", fd);
    long result;
    getdents_handler_impl(fd, reinterpret_cast<long>(entry), sizeof(linux_dirent64), &result, true);

    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        throw std::runtime_error(
            "ERROR: libcapio readdir on non CAPIO directory not yet supported");
    }

    LOG("\n\n");
    return result;
}

inline auto libcapio_mkdir(const char *path, int mode) {
    START_LOG(gettid(), "call(path=%s)", path);
    long result;
    mkdir_handler(reinterpret_cast<long>(path), mode, NULL, NULL, NULL, NULL, &result);

    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        throw std::runtime_error(
            "ERROR: libcapio readdir on non CAPIO directory not yet supported");
    }

    LOG("\n\n");
    return result;
}

inline auto libcapio_lseek(int fd, long offset, int whence) {
    START_LOG(gettid(), "call(fd=%d, offset=%d, whence=%d)", fd, offset, whence);
    long result;
    lseek_handler(fd, offset, whence, NULL, NULL, NULL, &result);

    if (result == CAPIO_POSIX_SYSCALL_REQUEST_SKIP) {
        throw std::runtime_error(
            "ERROR: libcapio readdir on non CAPIO directory not yet supported");
    }

    LOG("\n\n");
    return result;
}

#endif // PYCAPIO_PYCAPIO_HPP
