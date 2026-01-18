#include "libcapio.hpp"
#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "include/CapioTextIOWrapper.hpp"

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

    m.def("pycapio_get_capio_dir", &get_capio_dir, "Get capio directory");

    m.def("pycapio_open", &libcapio_open, "Open a file", pybind11::arg("path"),
          pybind11::arg("flags"), pybind11::arg("mode") = 0);

    pybind11::class_<CapioTextIOWrapper>(m, "PyCapioTextIOWrapper")
        .def(pybind11::init<int, uint64_t>(), pybind11::arg("fd"),
             pybind11::arg("chunk_size") = 4096)

        .def("read", &CapioTextIOWrapper::read, pybind11::arg("size") = -1)
        .def("readline", &CapioTextIOWrapper::readline)
        .def("readlines", &CapioTextIOWrapper::readlines)
        .def("write", &CapioTextIOWrapper::write, pybind11::arg("text"))
        .def("writelines", &CapioTextIOWrapper::writelines, pybind11::arg("lines"))
        .def("fileno", &CapioTextIOWrapper::fileno)
        .def("close", &CapioTextIOWrapper::close)
        .def("__enter__", [](CapioTextIOWrapper &self) -> CapioTextIOWrapper & { return self; })
        .def("__exit__",
             [](CapioTextIOWrapper &self, pybind11::object, pybind11::object, pybind11::object) {
                 self.close();
                 return false;
             });
}