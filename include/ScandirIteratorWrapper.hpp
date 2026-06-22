#ifndef LIBCAPIO_SCANDIR_ITERATOR_WRAPPER_HPP
#define LIBCAPIO_SCANDIR_ITERATOR_WRAPPER_HPP

#include <cstdint>
#include <dirent.h>
#include <filesystem>
#include <string>

/**
 * @brief A single entry produced while scanning a CAPIO directory.
 *
 * Mirrors the relevant parts of Python's @c os.DirEntry. Exposed to Python as
 * @c DirEntry and yielded by @c PyCapioScandirWrapper.
 */
class CapioDirEntry {
    std::filesystem::path base_path;
    std::string name_;
    ino64_t ino_;
    unsigned char type_;

  public:
    /// @brief Construct from a directory base path and a raw @c dirent64 record.
    /// @param base Directory that contains the entry.
    /// @param ent  Raw directory record returned by CAPIO.
    CapioDirEntry(const std::filesystem::path &base, const dirent64 &ent);

    /// @brief Entry name (final path component).
    [[nodiscard]] std::string name() const;

    /// @brief Full path of the entry (base path joined with the name).
    [[nodiscard]] std::string path() const;

    /// @brief Inode number of the entry.
    [[nodiscard]] uint64_t inode() const;

    /// @brief Return @c true if the entry is a directory.
    /// @param follow_symlinks Follow symbolic links when resolving the type.
    [[nodiscard]] bool is_dir(bool follow_symlinks = true) const;

    /// @brief Return @c true if the entry is a regular file.
    /// @param follow_symlinks Follow symbolic links when resolving the type.
    [[nodiscard]] bool is_file(bool follow_symlinks = true) const;

    /// @brief Return @c true if the entry is a symbolic link.
    [[nodiscard]] bool is_symlink() const;
};

/**
 * @brief Iterator over the contents of a CAPIO directory.
 *
 * Implements the Python iterator and context-manager protocols, yielding
 * @ref CapioDirEntry objects. Exposed to Python as @c PyCapioScandirWrapper and
 * used by the @c scandir / @c listdir proxies.
 */
class ScandirIteratorWrapper {
    std::filesystem::path path;
    int file_descriptor = -1;
    bool finished       = false;

  public:
    /// @brief Open @p path for scanning.
    explicit ScandirIteratorWrapper(const std::filesystem::path &path);

    /// @brief Close the underlying directory descriptor.
    ~ScandirIteratorWrapper();

    /// @brief Return the next entry, or raise @c StopIteration when exhausted.
    CapioDirEntry next();

    /// @brief Return @c *this so the object is its own iterator.
    ScandirIteratorWrapper &iter();

    /// @brief Release the directory descriptor early.
    void close();
};

#endif // LIBCAPIO_SCANDIR_ITERATOR_WRAPPER_HPP
