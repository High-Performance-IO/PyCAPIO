#include "libcapio.hpp"
#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "include/_CapioIOWrapper.hpp"
#include "include/_CapioScandirIteratorWrapper.hpp"

PYBIND11_MODULE(_pycapio, m) {

    m.doc() = "libcapio bindings for python"; // optional module docstring

    pybind11::dict file_modes;

    file_modes["O_RDONLY"] = O_RDONLY;
    file_modes["O_WRONLY"] = O_WRONLY;
    file_modes["O_RDWR"]   = O_RDWR;
    file_modes["O_CREAT"]  = O_CREAT;
    file_modes["O_APPEND"] = O_APPEND;

    m.attr("FILE_MODES") = file_modes;

    m.attr("CAPIO_DEFAULT_APP_NAME")      = CAPIO_DEFAULT_APP_NAME;
    m.attr("CAPIO_DEFAULT_WORKFLOW_NAME") = CAPIO_DEFAULT_WORKFLOW_NAME;

    m.add_object("_cleanup", pybind11::capsule([]() { libcapio_teardown(); }));

    m.def("pycapio_init", &libcapio_init, "Initialize libcapio", pybind11::arg("CAPIO_DIR") = ".",
          pybind11::arg("CAPIO_APP_NAME")               = CAPIO_DEFAULT_APP_NAME,
          pybind11::arg("CAPIO_WORKFLOW_NAME")          = CAPIO_DEFAULT_WORKFLOW_NAME,
          pybind11::arg("capio_server_exec_path")       = "capio_server",
          pybind11::arg("capio_cl_configuration_file")  = "",
          pybind11::arg("await_server_timeout_seconds") = 2);
    m.def("pycapio_teardown", &libcapio_teardown, "Teardown libcapio",
          pybind11::arg("teardown_server") = false);
    m.def("pycapio_get_capio_dir", &get_capio_dir, "Get capio directory");
    m.def("pycapio_open", &libcapio_open, "Open a file", pybind11::arg("path"),
          pybind11::arg("flags"), pybind11::arg("mode") = 0);

    m.def("pycapio_mkdir", &libcapio_mkdir, "Create a directory", pybind11::arg("path"),
          pybind11::arg("flags") = 0x777);

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
        .def("seek", &CapioTextIOWrapper::seek, pybind11::arg("offset") = 0,
             pybind11::arg("whence") = SEEK_SET)
        .def("__enter__", [](CapioTextIOWrapper &self) { return self; })
        .def("__exit__",
             [](CapioTextIOWrapper &self, [[maybe_unused]] const pybind11::object &exc_type,
                [[maybe_unused]] const pybind11::object &exc_val,
                [[maybe_unused]] const pybind11::object &exc_tb) { self.close(); });

    pybind11::class_<CapioBinaryIOWrapper>(m, "PyCapioBinaryIOWrapper")
        .def(pybind11::init<int, uint64_t>(), pybind11::arg("fd"),
             pybind11::arg("chunk_size") = 16 * 1024)
        .def(
            "read",
            [](CapioBinaryIOWrapper &self, const int64_t size) {
                return pybind11::bytes(self.read(size));
            },
            pybind11::arg("size") = -1)
        .def("readline",
             [](CapioBinaryIOWrapper &self) { return pybind11::bytes(self.readline()); })
        .def("readlines",
             [](CapioBinaryIOWrapper &self) {
                 std::vector<std::string> lines = self.readlines();
                 std::vector<pybind11::bytes> py_lines;
                 for (const auto &line : lines) {
                     py_lines.emplace_back(line);
                 }
                 return py_lines;
             })
        .def("write", [](CapioBinaryIOWrapper &self,
                         const pybind11::bytes &b) { return self.write(std::string(b)); })
        .def("writelines", &CapioBinaryIOWrapper::writelines)
        .def("fileno", &CapioBinaryIOWrapper::fileno)
        .def("close", &CapioBinaryIOWrapper::close)
        .def("closed", [](CapioBinaryIOWrapper &self) { self.close(); })
        .def("seek", &CapioBinaryIOWrapper::seek, pybind11::arg("offset") = 0,
             pybind11::arg("whence") = SEEK_SET)
        .def("__enter__", [](CapioBinaryIOWrapper &self) { return &self; })
        .def("__exit__", [](CapioBinaryIOWrapper &self, pybind11::args) { self.close(); });

    pybind11::class_<CapioDirEntry>(m, "DirEntry")
        .def_property_readonly("name", &CapioDirEntry::name)
        .def_property_readonly("path", &CapioDirEntry::path)
        .def("inode", &CapioDirEntry::inode)
        .def("is_dir", &CapioDirEntry::is_dir, pybind11::arg("follow_symlinks") = true)
        .def("is_file", &CapioDirEntry::is_file, pybind11::arg("follow_symlinks") = true)
        .def("is_symlink", &CapioDirEntry::is_symlink);

    pybind11::class_<_CapioScandirIteratorWrapper>(m, "PyCapioScandirWrapper")
        .def(pybind11::init<const std::filesystem::path &>())
        .def("__iter__", &_CapioScandirIteratorWrapper::iter,
             pybind11::return_value_policy::reference_internal)
        .def("__next__", &_CapioScandirIteratorWrapper::next)
        .def("close", &_CapioScandirIteratorWrapper::close)
        .def(
            "__enter__",
            [](_CapioScandirIteratorWrapper &self) -> _CapioScandirIteratorWrapper & {
                return self;
            },
            pybind11::return_value_policy::reference_internal)
        .def("__exit__", [](_CapioScandirIteratorWrapper &self, pybind11::object, pybind11::object,
                            pybind11::object) {
            self.close();
            return false;
        });
}
