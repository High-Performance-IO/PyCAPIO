#ifndef LIBCAPIO__CAPIOSCANDIRITERATORWRAPPER_HPP
#define LIBCAPIO__CAPIOSCANDIRITERATORWRAPPER_HPP
#include <dirent.h>

class _CapioScandirIteratorWrapper {
    const std::filesystem::path path;
    int file_descriptor = -1;

  public:
    _CapioScandirIteratorWrapper(const std::filesystem::path &path) : path(path) {
        if (!libcapio_initialized) {
            throw std::runtime_error("libcapio not initialized");
        }
        file_descriptor = libcapio_open(path.string().c_str(), O_RDONLY);
    }

    dirent64 next() {
        dirent64 directory_entry;
        if (libcapio_readdir(file_descriptor, &directory_entry) == NULL) {
            return {};
        }

        return next();
    }
};

#endif // LIBCAPIO__CAPIOSCANDIRITERATORWRAPPER_HPP
