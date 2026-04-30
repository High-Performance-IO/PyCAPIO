#ifndef LIBCAPIO_STACK_TRACE_HPP
#define LIBCAPIO_STACK_TRACE_HPP
#include <exception>
#include <iostream>
#include <ostream>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <string>

inline void trigger_stack_trace(const std::string &error_message) {
    const pybind11::object traceback = pybind11::module_::import("traceback");
    const pybind11::object io        = pybind11::module_::import("io");
    pybind11::object string_io       = io.attr("StringIO")();
    traceback.attr("print_stack")(pybind11::none(), pybind11::none(), string_io);
    const auto trace = string_io.attr("getvalue")().cast<std::string>();
    std::cout << "Python stack trace:\n" << trace << std::endl;

    throw std::runtime_error(error_message);
}
#endif // LIBCAPIO_STACK_TRACE_HPP
