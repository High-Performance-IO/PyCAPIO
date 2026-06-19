#include "PyCapioException.hpp"

#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

std::string PyCapioException::capture_python_stack() {
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

PyCapioException::PyCapioException(const std::string &error_message)
    : _message("PyCapioException: " + error_message + "\nPython stack trace:\n" +
               capture_python_stack()) {
    std::cerr << _message << std::endl;
}

const char *PyCapioException::what() const noexcept {
    return _message.c_str();
}