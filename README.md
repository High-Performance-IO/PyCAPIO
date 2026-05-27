# PyCAPIO

[![codecov](https://codecov.io/gh/High-Performance-IO/PyCAPIO/graph/badge.svg?token=YOUR_TOKEN)](https://codecov.io/gh/OWNER/REPO)
[![CI Tests](https://github.com/High-Performance-IO/PyCAPIO/actions/workflows/ci.yml/badge.svg)](https://github.com/High-Performance-IO/PyCAPIO/actions)

[![PyPI version](https://img.shields.io/pypi/v/pycapio.svg)](https://pypi.org/project/pycapio/)
[![Python Versions](https://img.shields.io/pypi/pyversions/pycapio.svg)](https://pypi.org/project/pycapio/)

**PyCAPIO** brings transparent data streaming to file-based Python workflows, minimizing I/O bottlenecks without
requiring code modifications.
PyCAPIO works by monkey patching built-in IO methods and classes, interfacing them natively with the CAPIO methodology.

---

## Prerequisites

Before installing PyCAPIO, ensure your system meets the following requirements:

| Dependency      | Minimum Version / Notes                              |
|:----------------|:-----------------------------------------------------|
| **Python**      | $\ge$ 3.10 (including development libraries/headers) |
| **MPI**         | A working MPI implementation (e.g., OpenMPI, MPICH)  |
| **Build Tools** | CMake & GCC compilers                                |

At Compile time, [CAPIO](https://github.com/High-Performance-IO/capio) is also fetched and compiled from source.

---

## Installation

### From PyPI (Recommended)

PyCAPIO is available as prebuilt wheels on PyPI. You can install it inside a virtual environment:

```bash
python3 -m venv venv
source venv/bin/activate
pip install pycapio
```

### From Source

To build and install PyCAPIO from the source repository:

```bash
git clone [https://github.com/High-Performance-IO/PyCAPIO.git](https://github.com/High-Performance-IO/PyCAPIO.git)
cd PyCAPIO
python3 -m venv venv
source venv/bin/activate
pip install .
```

## Usage

1. Global Interception (CLI)
   PyCAPIO can be invoked via the command line to inject streaming capabilities across your entire workflow step:
    ```bash
   pycapio --capio-dir ./dir --app-name app --workflow-name ./workflow my_step.py
   ```
2. Fine-Grained Interception (CapioContext)
   For precise control over which regions handle interception, use the CapioContext decorator. Only I/O operations
   executed inside this context will be intercepted by CAPIO.
    ```python
    from pycapio import CapioContext
    
    
    @CapioContext(capio_dir=".", app_name="reader", workflow_name="example_workflow")
    def read(path):
        with open(path, "r") as f:
            data = f.read()
            return data
    
    # Only I/O within this function call is intercepted
    data = read("example.txt")
    ```
3. PyCAPIO internal
   It is finally possible to use PyCAPIO's internal methods to intercept I/O operations. To do this, simply import the
   PyCAPIO internal module and use its methods.
    ```python
   from pycapio.internals import *
   ```
   This will import all the necessary methods and classes that can be used to develop custom IO modules which will
   interface directly with the CAPIO server.

# Important Notes

[!IMPORTANT] PyCAPIO natively intercepts I/O operations that use Python's built-in methods. If your script relies on
external libraries compiled into shared objects (.so files), please bypass Python-level interception and use the
standard LD_PRELOAD approach for CAPIO instead.

# Team

## Developing team

| Name                         | Role                    | Contact                                                                                                                        |
|------------------------------|-------------------------|--------------------------------------------------------------------------------------------------------------------------------|
| **Marco Edoardo Santimaria** | Designer and Maintainer | [email](mailto:marcoedoardo.santimaria@unito.it)  \| [Homepage](https://alpha.di.unito.it/marco-santimaria/)                   |
| **Rosa Filgueira**           | Workflows Expert        | [email](mailto:r.filgueira@ed.ac.uk)              \| [Homepage](https://www.epcc.ed.ac.uk/about-us/our-team/dr-rosa-filgueira) |
| **Iacopo Colonnelli**        | Workflows Expert        | [email](mailto:iacopo.colonnelli@unito.it)    \| [Homepage](https://alpha.di.unito.it/iacopo-colonnelli/)                      |
| **Marco Aldinucci**          | Supervisor              | [email](mailto:marco.aldinucci@unito.it)  \| [Homepage](https://alpha.di.unito.it/marco-aldinucci/)                            |
