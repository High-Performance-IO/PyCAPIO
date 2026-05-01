#ifndef LIBCAPIO__CAPIOOSPATH_HPP
#define LIBCAPIO__CAPIOOSPATH_HPP

#include <filesystem>
#include <string>

class OsPath {
  public:
    /**
     *   LIBCAPIO METHODS
     **/

    static bool exists(const std::string &path) {
        struct stat statbuf{};
        if (libcapio_stat(path.c_str(), &statbuf) == -1 && errno == ENOENT) {
            return false;
        }
        return true;
    }

    static bool isfile(const std::string &path) {
        struct stat statbuf{};
        if (libcapio_stat(path.c_str(), &statbuf) == -1 && errno == ENOENT) {
            return false;
        }

        return (statbuf.st_mode & S_IFMT) == S_IFREG;
    }

    static bool isdir(const std::string &path) { return !isfile(path); }

    static uintmax_t getsize(const std::string &path) {
        struct stat statbuf{};
        if (libcapio_stat(path.c_str(), &statbuf) == -1 && errno == ENOENT) {
            return 0;
        }
        return statbuf.st_size;
    }

    /**
     *  NON LIBCAPIO METHODS
     **/

    static std::string basename(const std::string &path) {
        return std::filesystem::path(path).filename().string();
    }

    static std::string dirname(const std::string &path) {
        return std::filesystem::path(path).parent_path().string();
    }

    static std::string abspath(const std::string &path) {
        return std::filesystem::absolute(path).string();
    }

    static std::pair<std::string, std::string> splitext(const std::string &path) {
        std::filesystem::path p(path);
        return {p.stem().string(), p.extension().string()};
    }

    static bool isabs(const std::string &p) { return std::filesystem::path(p).is_absolute(); }

    static std::string normpath(const std::string &p) {
        return std::filesystem::path(p).lexically_normal().string();
    }

    static std::string relpath(const std::string &path, const std::string &start = ".") {
        return std::filesystem::relative(path, start).string();
    }

    static double getmtime(const std::string &path) {
        auto ftime = std::filesystem::last_write_time(path);
        auto sctp  = std::chrono::time_point_cast<std::chrono::seconds>(ftime);
        return static_cast<double>(sctp.time_since_epoch().count());
    }

    static bool samefile(const std::string &p1, const std::string &p2) {
        return std::filesystem::equivalent(p1, p2);
    }

    static std::pair<std::string, std::string> split(const std::string &p) {
        std::filesystem::path path(p);
        return {path.parent_path().string(), path.filename().string()};
    }

    // On Unix, normcase simply returns the path unchanged.
    static std::string normcase(const std::string &p) { return p; }

    static std::string realpath(const std::string &path) {
        try {
            return std::filesystem::canonical(path).string();
        } catch (const std::filesystem::filesystem_error &) {
            return "";
        }
    }

    static std::string join(const std::string &path1, const std::string &path2) {
        if (path1.empty() && path2.empty()) {
            return "";
        }
        if (path1.empty() && !path2.empty()) {
            return path2;
        }
        if (path2.empty() && !path1.empty()) {
            return path1;
        }

        return std::filesystem::path(path1) / path2;
    }
};

#endif // LIBCAPIO__CAPIOOSPATH_HPP
