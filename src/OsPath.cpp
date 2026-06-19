#include <cerrno>
#include <chrono>
#include <filesystem>
#include <sys/stat.h>

#include "OsPath.hpp"
#include "libcapio.hpp"

// Note: Ensure that the header defining 'libcapio_stat' is included here
// or in your build environment. e.g., #include "libcapio.h"

bool OsPath::exists(const std::string &path) {
    struct stat statbuf{};
    if (libcapio_stat(path.c_str(), &statbuf) == -1 && errno == ENOENT) {
        return false;
    }
    return true;
}

bool OsPath::isfile(const std::string &path) {
    struct stat statbuf{};
    if (libcapio_stat(path.c_str(), &statbuf) == -1 && errno == ENOENT) {
        return false;
    }
    return (statbuf.st_mode & S_IFMT) == S_IFREG;
}

bool OsPath::isdir(const std::string &path) { return !isfile(path); }

uintmax_t OsPath::getsize(const std::string &path) {
    struct stat statbuf{};
    if (libcapio_stat(path.c_str(), &statbuf) == -1 && errno == ENOENT) {
        return 0;
    }
    return statbuf.st_size;
}

std::string OsPath::basename(const std::string &path) {
    return std::filesystem::path(path).filename().string();
}

std::string OsPath::dirname(const std::string &path) {
    return std::filesystem::path(path).parent_path().string();
}

std::string OsPath::abspath(const std::string &path) {
    return std::filesystem::absolute(path).string();
}

std::pair<std::string, std::string> OsPath::splitext(const std::string &path) {
    std::filesystem::path p(path);
    return {p.stem().string(), p.extension().string()};
}

bool OsPath::isabs(const std::string &p) { return std::filesystem::path(p).is_absolute(); }

std::string OsPath::normpath(const std::string &p) {
    return std::filesystem::path(p).lexically_normal().string();
}

std::string OsPath::relpath(const std::string &path, const std::string &start) {
    return std::filesystem::relative(path, start).string();
}

double OsPath::getmtime(const std::string &path) {
    auto ftime = std::filesystem::last_write_time(path);
    auto sctp  = std::chrono::time_point_cast<std::chrono::seconds>(ftime);
    return static_cast<double>(sctp.time_since_epoch().count());
}

bool OsPath::samefile(const std::string &p1, const std::string &p2) {
    return std::filesystem::equivalent(p1, p2);
}

std::pair<std::string, std::string> OsPath::split(const std::string &p) {
    std::filesystem::path path(p);
    return {path.parent_path().string(), path.filename().string()};
}

std::string OsPath::normcase(const std::string &p) { return p; }

std::string OsPath::realpath(const std::string &path) {
    try {
        return std::filesystem::canonical(path).string();
    } catch (const std::filesystem::filesystem_error &) {
        return "";
    }
}

std::string OsPath::join(const std::string &path1, const std::string &path2) {
    if (path1.empty() && path2.empty()) {
        return "";
    }
    if (path1.empty() && !path2.empty()) {
        return path2;
    }
    if (path2.empty() && !path1.empty()) {
        return path1;
    }

    return (std::filesystem::path(path1) / path2).string();
}