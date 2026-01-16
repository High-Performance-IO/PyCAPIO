#include "libcapio.hpp"
#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

PYBIND11_MODULE(_pycapio, m) {
    m.doc() = "libcapio bindings for python"; // optional module docstring

    pybind11::dict file_modes;

    file_modes["O_RDONLY"] = O_RDONLY;
    file_modes["O_WRONLY"] = O_WRONLY;
    file_modes["O_RDWR"]   = O_RDWR;
    file_modes["O_CREAT"]  = O_CREAT;
    file_modes["O_APPEND"] = O_APPEND;

    m.attr("FILE_MODES") = file_modes;

    m.def("pycapio_init", &libcapio_init, "Initialize libcapio", pybind11::arg("CAPIO_DIR") = ".",
          pybind11::arg("CAPIO_APP_NAME")      = CAPIO_DEFAULT_APP_NAME,
          pybind11::arg("CAPIO_WORKFLOW_NAME") = CAPIO_DEFAULT_WORKFLOW_NAME);

    m.def("pycapio_teardown", &libcapio_teardown, "Teardown libcapio");

    m.def("pycapio_open", &libcapio_open, "Open a file", pybind11::arg("path"),
          pybind11::arg("flags"), pybind11::arg("mode") = 0);

    m.def("pycapio_close", &libcapio_close, "Close a file", pybind11::arg("fd"));

    m.def("pycapio_read", &libcapio_read, "Read from a file", pybind11::arg("fd"),
          pybind11::arg("buf"), pybind11::arg("size"));

    m.def("pycapio_write", &libcapio_write, "Write to a file", pybind11::arg("fd"),
          pybind11::arg("buf"), pybind11::arg("size"));
}