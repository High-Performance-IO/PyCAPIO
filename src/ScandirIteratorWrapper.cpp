#include <cstring>
#include <fcntl.h>
#include <pybind11/pybind11.h>

#include "PyCapioException.hpp"
#include "ScandirIteratorWrapper.hpp"
#include "libcapio.hpp"

extern int capio_server_thread_id;
extern bool libcapio_initialized;

// -----------------------------------------------------------------------------
// CapioDirEntry Implementation
// -----------------------------------------------------------------------------

CapioDirEntry::CapioDirEntry(const std::filesystem::path &base, const dirent64 &ent)
    : base_path(base), name_(ent.d_name), ino_(ent.d_ino), type_(ent.d_type) {}

std::string CapioDirEntry::name() const { return name_; }

std::string CapioDirEntry::path() const { return (base_path / name_).string(); }

uint64_t CapioDirEntry::inode() const { return ino_; }

bool CapioDirEntry::is_dir([[maybe_unused]] bool follow_symlinks) const { return type_ == DT_DIR; }

bool CapioDirEntry::is_file([[maybe_unused]] bool follow_symlinks) const { return type_ == DT_REG; }

bool CapioDirEntry::is_symlink() const { return type_ == DT_LNK; }

// -----------------------------------------------------------------------------
// ScandirIteratorWrapper Implementation
// -----------------------------------------------------------------------------

ScandirIteratorWrapper::ScandirIteratorWrapper(const std::filesystem::path &path) : path(path) {
    if (!libcapio_initialized) {
        throw PyCapioException("libcapio not initialized");
    }

    file_descriptor = libcapio_open(path.string().c_str(), O_RDONLY);
    if (file_descriptor < 0) {
        throw PyCapioException("libcapio_open failed");
    }
}

ScandirIteratorWrapper::~ScandirIteratorWrapper() { close(); }

CapioDirEntry ScandirIteratorWrapper::next() {
    if (finished) {
        throw pybind11::stop_iteration();
    }

    dirent64 ent{};
    if (libcapio_readdir(file_descriptor, &ent) == 0) {
        finished = true;
        throw pybind11::stop_iteration();
    }

    // Skip '.' and '..'
    if (strcmp(ent.d_name, ".") == 0 || strcmp(ent.d_name, "..") == 0) {
        return next();
    }

    return {path, ent};
}

ScandirIteratorWrapper &ScandirIteratorWrapper::iter() { return *this; }

void ScandirIteratorWrapper::close() {
    if (file_descriptor != -1) {
        libcapio_close(file_descriptor);
        file_descriptor = -1;
    }
}