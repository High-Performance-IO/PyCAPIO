#ifndef LIBCAPIO__CAPIOSCANDIRITERATORWRAPPER_HPP
#define LIBCAPIO__CAPIOSCANDIRITERATORWRAPPER_HPP
#include <dirent.h>

class CapioDirEntry {
    std::filesystem::path base_path;
    std::string name_;
    ino64_t ino_;
    unsigned char type_;

  public:
    CapioDirEntry(const std::filesystem::path &base, const dirent64 &ent)
        : base_path(base), name_(ent.d_name), ino_(ent.d_ino), type_(ent.d_type) {}

    std::string name() const { return name_; }

    std::string path() const { return (base_path / name_).string(); }

    uint64_t inode() const { return ino_; }

    bool is_dir(bool follow_symlinks = true) const { return type_ == DT_DIR; }

    bool is_file(bool follow_symlinks = true) const { return type_ == DT_REG; }

    bool is_symlink() const { return type_ == DT_LNK; }
};

class _CapioScandirIteratorWrapper {
    std::filesystem::path path;
    int file_descriptor = -1;
    bool finished       = false;

  public:
    _CapioScandirIteratorWrapper(const std::filesystem::path &path) : path(path) {

        if (capio_server_thread_id <= 0) {
            throw std::runtime_error("libcapio not initialized");
        }

        file_descriptor = libcapio_open(path.string().c_str(), O_RDONLY);
        if (file_descriptor < 0) {
            throw std::runtime_error("libcapio_open failed");
        }
    }

    ~_CapioScandirIteratorWrapper() { close(); }

    CapioDirEntry next() {
        if (finished) {
            throw pybind11::stop_iteration();
        }

        dirent64 ent{};
        if (libcapio_readdir(file_descriptor, &ent) == NULL) {
            finished = true;
            throw pybind11::stop_iteration();
        }

        if (strcmp(ent.d_name, ".") == 0 || strcmp(ent.d_name, "..") == 0) {
            return next();
        }

        return {path, ent};
    }

    _CapioScandirIteratorWrapper &iter() { return *this; }

    void close() {
        if (file_descriptor != -1) {
            libcapio_close(file_descriptor);
            file_descriptor = -1;
        }
    }
};

#endif // LIBCAPIO__CAPIOSCANDIRITERATORWRAPPER_HPP
