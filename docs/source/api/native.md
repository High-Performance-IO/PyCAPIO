# Native API

Several names exported from {doc}`internals` (and re-exported at the top level)
are implemented in C++ and exposed to Python through pybind11 — the compiled
`pycapio._pycapio` extension. The table below maps each Python name to the C++
entity that implements it; follow the links for the full, source-generated
reference on the {doc}`../cpp/index` page.

| Python name              | C++ entity                  | Reference |
| ------------------------ | --------------------------- | --------- |
| `PyCapioTextIOWrapper`   | `IOWrapper<IOMode::Text>`   | {cpp:class}`IOWrapper` |
| `PyCapioBinaryIOWrapper` | `IOWrapper<IOMode::Binary>` | {cpp:class}`IOWrapper` |
| `PyCapioPath`            | `OsPath`                    | {cpp:class}`OsPath` |
| `DirEntry`               | `CapioDirEntry`             | {cpp:class}`CapioDirEntry` |
| `PyCapioScandirWrapper`  | `ScandirIteratorWrapper`    | {cpp:class}`ScandirIteratorWrapper` |
| `PyCAPIOException`        | `PyCapioException`          | {cpp:class}`PyCapioException` |

| Python function                       | C++ function          | Reference |
| ------------------------------------- | --------------------- | --------- |
| `pycapio_init`                        | `libcapio_init`       | {cpp:func}`libcapio_init` |
| `pycapio_teardown`                    | `libcapio_teardown`   | {cpp:func}`libcapio_teardown` |
| `pycapio_open`                        | `libcapio_open`       | {cpp:func}`libcapio_open` |
| `pycapio_mkdir`                       | `libcapio_mkdir`      | {cpp:func}`libcapio_mkdir` |

## Behaviour summary

**IO wrappers.** `PyCapioTextIOWrapper` and `PyCapioBinaryIOWrapper` are the
`Text` and `Binary` instantiations of the same `IOWrapper` class template. They
implement the file-object protocol (`read`, `readline`, `readlines`, `write`,
`writelines`, `seek`, `flush`, `close`, iteration and context-manager support)
on top of a CAPIO file descriptor. Text wrappers return `str`; binary wrappers
return `bytes`.

**`PyCapioPath`.** A drop-in replacement for {mod}`os.path` whose queries are
answered by CAPIO for paths inside the CAPIO directory, installed in place of
`os.path` while a `CapioContext` is active.

**Directory scanning.** `PyCapioScandirWrapper` iterates a CAPIO directory and
yields `DirEntry` objects, mirroring {func}`os.scandir` and `os.DirEntry`.

The full rendered reference for all of these lives on the {doc}`../cpp/index`
page.
