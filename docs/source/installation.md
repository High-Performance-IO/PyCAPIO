# Installation

## Prerequisites

Before installing PyCAPIO, ensure your system meets the following requirements:

| Dependency      | Minimum version / notes                                |
| --------------- | ------------------------------------------------------ |
| **Python**      | ≥ 3.10 (including development libraries / headers)      |
| **MPI**         | A working MPI implementation (e.g. OpenMPI, MPICH)      |
| **Build tools** | CMake and GCC                                          |

At compile time, [CAPIO](https://github.com/High-Performance-IO/capio) is also
fetched and compiled from source.

## From PyPI (recommended)

PyCAPIO is published as prebuilt wheels on PyPI. Install it inside a virtual
environment:

```console
python3 -m venv venv
source venv/bin/activate
pip install pycapio
```

## From source

To build and install PyCAPIO from the repository:

```console
git clone https://github.com/High-Performance-IO/PyCAPIO.git
cd PyCAPIO
python3 -m venv venv
source venv/bin/activate
pip install .
```

:::{important}
PyCAPIO natively intercepts I/O operations that use Python's built-in methods.
If your script relies on external libraries compiled into shared objects
(`.so` files), bypass Python-level interception and use the standard
`LD_PRELOAD` approach for CAPIO instead.
:::
