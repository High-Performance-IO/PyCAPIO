#include "libcapio.hpp"
#include "pybind11/pybind11.h"
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "include/stack_trace.hpp"

#include "include/IOWrapper.hpp"
#include "include/OsPath.hpp"
#include "include/ScandirIteratorWrapper.hpp"

namespace py11 = pybind11;

template <IOMode Mode>
void register_IO_wrapper(py11::module &m, const char *name, uint64_t default_chunk) {
    using W = IOWrapper<Mode>;
    py11::class_<W>(m, name)
        .def(py11::init<int, uint64_t>(), py11::arg("fd"), py11::arg("chunk_size") = default_chunk)
        .def("read", &W::read, py11::arg("size") = -1)
        .def("readline", &W::readline)
        .def("readlines", &W::readlines)
        .def("write", &W::write, py11::arg("text"))
        .def("writelines", &W::writelines, py11::arg("lines"))
        .def("fileno", &W::fileno)
        .def("close", &W::close)
        .def("flush", [](W &self) { self.flush(); })
        .def("seek", &W::seek, py11::arg("offset") = 0, py11::arg("whence") = SEEK_SET)
        .def("closed", [](W &self) { return self.closed(); })
        .def("__iter__", &W::iter, py11::return_value_policy::reference)
        .def("__next__", &W::next)
        .def("__enter__", [](W &self) { return &self; })
        .def("__exit__", [](W &self, py11::args) { self.close(); });
}

PYBIND11_MODULE(_pycapio, m) {

    m.doc() = "libcapio bindings for python";

    py11::dict file_modes;

    file_modes["O_RDONLY"] = O_RDONLY;
    file_modes["O_WRONLY"] = O_WRONLY;
    file_modes["O_RDWR"]   = O_RDWR;
    file_modes["O_CREAT"]  = O_CREAT;
    file_modes["O_APPEND"] = O_APPEND;

    m.attr("FILE_MODES") = file_modes;

    m.attr("CAPIO_DEFAULT_APP_NAME")      = CAPIO_DEFAULT_APP_NAME;
    m.attr("CAPIO_DEFAULT_WORKFLOW_NAME") = CAPIO_DEFAULT_WORKFLOW_NAME;

    m.add_object("_cleanup", py11::capsule([]() { libcapio_teardown(); }));

    m.def("pycapio_init", &libcapio_init, "Initialize libcapio", py11::arg("CAPIO_DIR") = ".",
          py11::arg("CAPIO_APP_NAME")               = CAPIO_DEFAULT_APP_NAME,
          py11::arg("CAPIO_WORKFLOW_NAME")          = CAPIO_DEFAULT_WORKFLOW_NAME,
          py11::arg("capio_server_exec_path")       = "capio_server",
          py11::arg("capio_cl_configuration_file")  = "",
          py11::arg("await_server_timeout_seconds") = 2);
    m.def("pycapio_teardown", &libcapio_teardown, "Teardown libcapio",
          py11::arg("teardown_server") = false);
    m.def("pycapio_get_capio_dir", &get_capio_dir, "Get capio directory");
    m.def("pycapio_open", &libcapio_open, "Open a file", py11::arg("path"), py11::arg("flags"),
          py11::arg("mode") = 0);

    m.def("pycapio_mkdir", &libcapio_mkdir, "Create a directory", py11::arg("path"),
          py11::arg("flags") = 0x777);

    register_IO_wrapper<IOMode::Text>(m, "PyCapioTextIOWrapper", 4096);
    register_IO_wrapper<IOMode::Binary>(m, "PyCapioBinaryIOWrapper", 16 * 1024);

    py11::class_<CapioDirEntry>(m, "DirEntry")
        .def_property_readonly("name", &CapioDirEntry::name)
        .def_property_readonly("path", &CapioDirEntry::path)
        .def("inode", &CapioDirEntry::inode)
        .def("is_dir", &CapioDirEntry::is_dir, py11::arg("follow_symlinks") = true)
        .def("is_file", &CapioDirEntry::is_file, py11::arg("follow_symlinks") = true)
        .def("is_symlink", &CapioDirEntry::is_symlink);

    py11::class_<ScandirIteratorWrapper>(m, "PyCapioScandirWrapper")
        .def(py11::init<const std::filesystem::path &>())
        .def("__iter__", &ScandirIteratorWrapper::iter,
             py11::return_value_policy::reference_internal)
        .def("__next__", &ScandirIteratorWrapper::next)
        .def("close", &ScandirIteratorWrapper::close)
        .def(
            "__enter__", [](ScandirIteratorWrapper &self) { return &self; },
            py11::return_value_policy::reference_internal)
        .def("__exit__", [](ScandirIteratorWrapper &self, py11::args) {
            self.close();
            return false;
        });

    py11::class_<OsPath>(m, "PyCapioPath")
        .def_static("exists", &OsPath::exists, py11::arg("path"))
        .def_static("isfile", &OsPath::isfile, py11::arg("path"))
        .def_static("isdir", &OsPath::isdir, py11::arg("path"))
        .def_static("getsize", &OsPath::getsize, py11::arg("path"))
        .def_static("join", &OsPath::join)
        .def_static("basename", &OsPath::basename, py11::arg("path"))
        .def_static("dirname", &OsPath::dirname, py11::arg("path"))
        .def_static("abspath", &OsPath::abspath, py11::arg("path"))
        .def_static("splitext", &OsPath::splitext, py11::arg("path"))
        .def_static("isabs", &OsPath::isabs, py11::arg("path"))
        .def_static("normpath", &OsPath::normpath, py11::arg("path"))
        .def_static("relpath", &OsPath::relpath, py11::arg("path"), py11::arg("start") = ".")
        .def_static("getmtime", &OsPath::getmtime, py11::arg("path"))
        .def_static("samefile", &OsPath::samefile, py11::arg("p1"), py11::arg("p2"))
        .def_static("split", &OsPath::split, py11::arg("path"))
        .def_static("normcase", &OsPath::normcase, py11::arg("path"))
        .def_static("realpath", &OsPath::realpath, py11::arg("path"));
}
