#pragma once

#include <cstdint>
#include <string>
#include <utility>

/**
 * @brief CAPIO-aware replacement for Python's @c os.path module.
 *
 * Each static method mirrors the semantics of the corresponding
 * @c os.path function. Methods in the *libcapio* group answer their query
 * through the CAPIO server (so they work on files that live only inside the
 * CAPIO directory); the remaining methods are pure path-string manipulation
 * that match the behaviour of the standard library.
 *
 * On the Python side this class is exposed as @c PyCapioPath and installed in
 * place of @c os.path while a @c CapioContext is active.
 */
class OsPath {
  public:
    /**
     * @name CAPIO-backed queries
     * These consult the CAPIO server and therefore see files managed by CAPIO.
     * @{
     */

    /// @brief Return @c true if @p path exists.
    /// @param path Path to test.
    static bool exists(const std::string &path);

    /// @brief Return @c true if @p path exists and is a regular file.
    /// @param path Path to test.
    static bool isfile(const std::string &path);

    /// @brief Return @c true if @p path exists and is a directory.
    /// @param path Path to test.
    static bool isdir(const std::string &path);

    /// @brief Return the size of @p path in bytes.
    /// @param path File whose size is requested.
    static uintmax_t getsize(const std::string &path);
    /** @} */

    /**
     * @name Pure path manipulation
     * String-only helpers that do not touch the filesystem.
     * @{
     */

    /// @brief Return the final component of @p path.
    static std::string basename(const std::string &path);

    /// @brief Return everything in @p path except the final component.
    static std::string dirname(const std::string &path);

    /// @brief Return the absolute, normalised form of @p path.
    static std::string abspath(const std::string &path);

    /// @brief Split @p path into a @c (root, extension) pair.
    static std::pair<std::string, std::string> splitext(const std::string &path);

    /// @brief Return @c true if @p p is an absolute path.
    static bool isabs(const std::string &p);

    /// @brief Collapse redundant separators and up-level references in @p p.
    static std::string normpath(const std::string &p);

    /// @brief Return @p path relative to @p start.
    /// @param path Target path.
    /// @param start Base directory the result is computed against (default ".").
    static std::string relpath(const std::string &path, const std::string &start = ".");

    /// @brief Return the last-modification time of @p path as a POSIX timestamp.
    static double getmtime(const std::string &path);

    /// @brief Return @c true if @p p1 and @p p2 refer to the same file.
    static bool samefile(const std::string &p1, const std::string &p2);

    /// @brief Split @p p into a @c (head, tail) pair at the last separator.
    static std::pair<std::string, std::string> split(const std::string &p);

    /// @brief Normalise the case of @p p (platform dependent).
    static std::string normcase(const std::string &p);

    /// @brief Resolve symbolic links in @p path to a canonical path.
    static std::string realpath(const std::string &path);

    /// @brief Join @p path1 and @p path2 with the path separator.
    static std::string join(const std::string &path1, const std::string &path2);
    /** @} */
};
