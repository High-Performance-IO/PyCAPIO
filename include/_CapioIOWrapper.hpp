#ifndef LIBCAPIO_PYCAPIOTEXTIOWRAPPER_HPP
#define LIBCAPIO_PYCAPIOTEXTIOWRAPPER_HPP
#include <cstdint>

#ifndef PYCAPIO_BINDINGS

namespace pybind11 {
class stop_iteration : public std::exception {
  public:
    stop_iteration()           = default;
    ~stop_iteration() override = default;
};
} // namespace pybind11
#endif

// TODO: make this class a template
// TODO: readline and readlines to use _CapioIOWrapperStopIteration
class _CapioIOWrapper {

    const int _file_descriptor = -1;
    const uint64_t _chunk_size;
    std::string _buffer;
    bool _closed    = false;
    bool _exhausted = false;

  public:
    // Default read of 16KB
    _CapioIOWrapper(const int fd, const uint64_t chunk_size = 16 * 1024)
        : _file_descriptor(fd), _chunk_size(chunk_size) {}

    [[nodiscard]] auto write(const std::string &text) const {
        const auto write_size = libcapio_write(_file_descriptor, text.data(), text.size());
        if (write_size != text.size()) {
            trigger_stack_trace("write failed: received from libcapio offset: " +
                                std::to_string(write_size));
        }
        return write_size;
    }

    auto writelines(const std::vector<std::string> &lines) const {
        for (const auto &line : lines) {
            write(line);
        }
    }

    std::string read(int64_t size = -1) {
        std::string out_buffer;
        size_t read_size = 0;

        if (size == 0) {
            return "";
        }

        out_buffer += _buffer;
        read_size += _buffer.size();
        _buffer.clear();

        if (size != -1) {
            out_buffer.resize(size);
            const auto result =
                libcapio_read(this->_file_descriptor, out_buffer.data() + read_size, size);
            read_size += result;
            out_buffer.resize(read_size);
            return out_buffer;
        }

        long long cnt = 0;
        std::string tmp("\0", _chunk_size);
        do {
            cnt = libcapio_read(this->_file_descriptor, tmp.data(), _chunk_size);
            out_buffer.append(tmp, 0, cnt);
            read_size += cnt;
        } while (cnt > 0);

        out_buffer.resize(read_size);
        return out_buffer;
    }

    std::string readline() {
        std::string out;

        while (true) {
            if (const auto nl = _buffer.find('\n'); nl != std::string::npos) {
                out.append(_buffer, 0, nl + 1);
                _buffer.erase(0, nl + 1);
                return out;
            }

            if (!_buffer.empty()) {
                out += _buffer;
                _buffer.clear();
            }

            std::string tmp;
            tmp.resize(_chunk_size);

            const ssize_t n = libcapio_read(_file_descriptor, tmp.data(), _chunk_size);

            if (n <= 0) {
                return out;
            }

            tmp.resize(n);
            _buffer += tmp;
        }
    }

    std::string next() {
        if (_exhausted) {
            throw pybind11::stop_iteration();
        }
        std::string line = readline();
        if (line.empty()) {
            _exhausted = true;
            throw pybind11::stop_iteration();
        }
        return line;
    }

    std::vector<std::string> readlines() {
        std::vector<std::string> lines;
        while (true) {
            lines.push_back(readline());
            if (lines.back().empty()) {
                return lines;
            }
        }
    }

    [[nodiscard]] auto fileno() const { return _file_descriptor; }

    void close() {
        if (!_closed) {
            libcapio_close(_file_descriptor);
            _closed = true;
        }
    }

    [[nodiscard]] auto seek(int offset, int whence) const {
        return libcapio_lseek(this->fileno(), offset, whence);
    }

    static void flush() {
        read_cache->flush();
        write_cache->flush();
    }

    [[nodiscard]] bool closed() const { return this->_closed; }
};

class CapioBinaryIOWrapper final : public _CapioIOWrapper {
    using _CapioIOWrapper::_CapioIOWrapper;

  public:
    ~CapioBinaryIOWrapper() {
        flush();
        close();
    }
    CapioBinaryIOWrapper &iter() { return *this; }
};

class CapioTextIOWrapper final : public _CapioIOWrapper {
    using _CapioIOWrapper::_CapioIOWrapper;

  public:
    ~CapioTextIOWrapper() {
        flush();
        close();
    }
    CapioTextIOWrapper &iter() { return *this; }
};

#endif // LIBCAPIO_PYCAPIOTEXTIOWRAPPER_HPP
