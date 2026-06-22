# PyCAPIO

```{toctree}
:maxdepth: 2
:hidden:
:caption: Getting started

installation
usage
```

```{toctree}
:maxdepth: 2
:hidden:
:caption: Python API

api/pycapio
api/internals
api/native
```

```{toctree}
:maxdepth: 2
:hidden:
:caption: C++ API

cpp/index
```

**PyCAPIO** brings transparent data streaming to file-based Python workflows,
minimizing I/O bottlenecks without requiring code modifications. It works by
*monkey patching* Python's built-in I/O methods and classes, interfacing them
natively with the [CAPIO](https://github.com/High-Performance-IO/capio)
methodology.

When a workflow step runs under PyCAPIO, any access to a path inside the
configured **CAPIO directory** is routed through the CAPIO server instead of the
operating system, while all other I/O behaves exactly as it normally would.

## Where to start

- {doc}`installation` — prerequisites and how to install PyCAPIO.
- {doc}`usage` — the three ways to enable interception (CLI, `CapioContext`,
  and the low-level internals).
- {doc}`api/pycapio` — the high-level Python API.
- {doc}`api/native` and {doc}`cpp/index` — the native (C++/pybind11) layer.

## At a glance

```python
from pycapio import CapioContext


@CapioContext(capio_dir=".", app_name="reader", workflow_name="example_workflow")
def read(path):
    with open(path, "r") as f:
        return f.read()


# Only the I/O performed inside this call is intercepted by CAPIO.
data = read("example.txt")
```

## Indices

- {ref}`genindex`
- {ref}`modindex`
- {ref}`search`
