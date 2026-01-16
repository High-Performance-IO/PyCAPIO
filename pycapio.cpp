#include "libcapio.hpp"
#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

PYBIND11_MODULE(_pycapio, m) {
    m.doc() = "libcapio bindings for python"; // optional module docstring

    m.def("pycapio_init", &libcapio_init, "Initialize libcapio");
    m.def("pycapio_teardown", &libcapio_teardown, "Teardown libcapio");
    m.def("pycapio_open", &libcapio_open, "Open a file");
    m.def("pycapio_close", &libcapio_close, "Close a file");
    m.def("pycapio_read", &libcapio_read, "Read from a file");
    m.def("pycapio_write", &libcapio_write, "Write to a file");
}