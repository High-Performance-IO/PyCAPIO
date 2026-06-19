#pragma once

#include <cstdint>
#include <string>
#include <utility>

class OsPath {
  public:
    /**
     * LIBCAPIO METHODS
     **/
    static bool exists(const std::string &path);
    static bool isfile(const std::string &path);
    static bool isdir(const std::string &path);
    static uintmax_t getsize(const std::string &path);

    /**
     * NON LIBCAPIO METHODS
     **/
    static std::string basename(const std::string &path);
    static std::string dirname(const std::string &path);
    static std::string abspath(const std::string &path);
    static std::pair<std::string, std::string> splitext(const std::string &path);
    static bool isabs(const std::string &p);
    static std::string normpath(const std::string &p);
    static std::string relpath(const std::string &path, const std::string &start = ".");
    static double getmtime(const std::string &path);
    static bool samefile(const std::string &p1, const std::string &p2);
    static std::pair<std::string, std::string> split(const std::string &p);
    static std::string normcase(const std::string &p);
    static std::string realpath(const std::string &path);
    static std::string join(const std::string &path1, const std::string &path2);
};