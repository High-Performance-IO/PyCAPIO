#ifndef LIBCAPIO_IOWRAPPER_HPP
#define LIBCAPIO_IOWRAPPER_HPP
#include <cstdint>

enum class IOMode { Text, Binary };

template <IOMode Mode> class IOWrapper {

    const int _file_descriptor = -1;
    const uint64_t _chunk_size;
    std::string _buffer;
    bool _closed    = false;
    bool _exhausted = false;

    using ReturnType = std::conditional_t<Mode == IOMode::Text, std::string, pybind11::bytes>;

    static ReturnType wrap(std::string s) {
        if constexpr (Mode == IOMode::Text) {
            return s;
        } else {
            return pybind11::bytes(s);
        }
    }

    std::string readline_raw() {
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

  public:
    IOWrapper(const int fd, const uint64_t chunk_size = 16 * 1024)
        : _file_descriptor(fd), _chunk_size(chunk_size) {}

    ~IOWrapper() {
        flush();
        close();
    }

    auto write(const std::string &text) const {
        const auto write_size = libcapio_write(_file_descriptor, text.data(), text.size());
        if (write_size != static_cast<ssize_t>(text.size())) {
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

    ReturnType read(int64_t size = -1) {
        std::string out_buffer;
        size_t read_size = 0;

        if (size == 0) {
            return wrap("");
        }

        out_buffer += _buffer;
        read_size += _buffer.size();
        _buffer.clear();

        if (size != -1) {
            out_buffer.resize(size);
            const auto result =
                libcapio_read(_file_descriptor, out_buffer.data() + read_size, size);
            read_size += result;
            out_buffer.resize(read_size);
            return wrap(std::move(out_buffer));
        }

        long long cnt = 0;
        std::string tmp("\0", _chunk_size);
        do {
            cnt = libcapio_read(_file_descriptor, tmp.data(), _chunk_size);
            out_buffer.append(tmp, 0, cnt);
            read_size += cnt;
        } while (cnt > 0);

        out_buffer.resize(read_size);
        return wrap(std::move(out_buffer));
    }

    ReturnType readline() {
        std::string out;
        while (true) {
            if (const auto nl = _buffer.find('\n'); nl != std::string::npos) {
                out.append(_buffer, 0, nl + 1);
                _buffer.erase(0, nl + 1);
                return wrap(std::move(out));
            }
            if (!_buffer.empty()) {
                out += _buffer;
                _buffer.clear();
            }
            std::string tmp;
            tmp.resize(_chunk_size);
            const ssize_t n = libcapio_read(_file_descriptor, tmp.data(), _chunk_size);
            if (n <= 0) {
                return wrap(std::move(out));
            }
            tmp.resize(n);
            _buffer += tmp;
        }
    }

    ReturnType next() {
        if (_exhausted) {
            throw pybind11::stop_iteration();
        }

        std::string line = readline_raw();
        if (line.empty()) {
            _exhausted = true;
            throw pybind11::stop_iteration();
        }
        return wrap(std::move(line));
    }

    std::vector<ReturnType> readlines() {
        std::vector<ReturnType> lines;
        try {
            while (true) {
                lines.push_back(next());
            }
        } catch (const pybind11::stop_iteration &) {
        }
        return lines;
    }

    [[nodiscard]] auto fileno() const { return _file_descriptor; }

    void close() {
        if (!_closed) {
            libcapio_close(_file_descriptor);
            _closed = true;
        }
    }

    [[nodiscard]] auto seek(const int offset, const int whence) const {
        return libcapio_lseek(fileno(), offset, whence);
    }

    static void flush() {
        read_cache->flush();
        write_cache->flush();
    }

    [[nodiscard]] bool closed() const { return _closed; }

    IOWrapper &iter() { return *this; }
};

using CapioTextIOWrapper   = IOWrapper<IOMode::Text>;
using CapioBinaryIOWrapper = IOWrapper<IOMode::Binary>;

#endif // LIBCAPIO_IOWRAPPER_HPP
