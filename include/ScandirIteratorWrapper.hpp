#ifndef LIBCAPIO_SCANDIR_ITERATOR_WRAPPER_HPP
#define LIBCAPIO_SCANDIR_ITERATOR_WRAPPER_HPP

#include <cstdint>
#include <dirent.h>
#include <filesystem>
#include <string>

class CapioDirEntry {
    std::filesystem::path base_path;
    std::string name_;
    ino64_t ino_;
    unsigned char type_;

  public:
    CapioDirEntry(const std::filesystem::path &base, const dirent64 &ent);

    [[nodiscard]] std::string name() const;
    [[nodiscard]] std::string path() const;
    [[nodiscard]] uint64_t inode() const;
    [[nodiscard]] bool is_dir(bool follow_symlinks = true) const;
    [[nodiscard]] bool is_file(bool follow_symlinks = true) const;
    [[nodiscard]] bool is_symlink() const;
};

class ScandirIteratorWrapper {
    std::filesystem::path path;
    int file_descriptor = -1;
    bool finished       = false;

  public:
    explicit ScandirIteratorWrapper(const std::filesystem::path &path);
    ~ScandirIteratorWrapper();

    CapioDirEntry next();
    ScandirIteratorWrapper &iter();
    void close();
};

#endif // LIBCAPIO_SCANDIR_ITERATOR_WRAPPER_HPP