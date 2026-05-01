#ifndef PYCAPIO_EXCEPTION_HPP
#define PYCAPIO_EXCEPTION_HPP
#include <exception>
#include <iostream>
#include <ostream>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <string>

class PyCapioException : public std::exception {
    std::string _message;

    static std::string capture_python_stack() {
        try {
            const auto traceback = pybind11::module_::import("traceback");
            const auto io        = pybind11::module_::import("io");
            auto string_io       = io.attr("StringIO")();
            traceback.attr("print_stack")(pybind11::none(), pybind11::none(), string_io);
            return string_io.attr("getvalue")().cast<std::string>();
        } catch (const pybind11::error_already_set &e) {
            return std::string("[failed to capture Python stack: ") + e.what() + "]";
        } catch (...) {
            return "[failed to capture Python stack: unknown error]";
        }
    }

  public:
    explicit PyCapioException(const std::string &error_message)
        : _message("PyCapioException: " + error_message + "\nPython stack trace:\n" +
                   capture_python_stack()) {
        std::cerr << _message << std::endl;
    }

    [[nodiscard]] const char *what() const noexcept override { return _message.c_str(); }
};
#endif // PYCAPIO_EXCEPTION_HPP
