# Usage

PyCAPIO can inject transparent streaming at three levels of granularity.

## 1. Global interception (CLI)

Invoke PyCAPIO from the command line to inject streaming across an entire
workflow step. The target script runs unmodified; every built-in I/O call it
makes against the CAPIO directory is intercepted.

```console
pycapio --capio-dir ./dir --app-name app --workflow-name my_workflow my_step.py
```

Available options are documented in {func}`pycapio.__main__.main`. Any arguments
after the script path are forwarded verbatim to the script.

## 2. Fine-grained interception (`CapioContext`)

For precise control over which regions are intercepted, use the
{func}`pycapio.CapioContext` decorator. Only I/O performed *inside* the decorated
call is routed through CAPIO.

```python
from pycapio import CapioContext


@CapioContext(capio_dir=".", app_name="reader", workflow_name="example_workflow")
def read(path):
    with open(path, "r") as f:
        data = f.read()
        return data


# Only I/O within this function call is intercepted.
data = read("example.txt")
```

The decorator initialises CAPIO on first use, patches the built-in I/O callables
for the duration of the call, and restores the originals afterwards. See the
{doc}`api/pycapio` reference for every parameter.

## 3. Low-level internals

You can also drive CAPIO directly through its primitives to build custom I/O
modules that interface with the CAPIO server:

```python
from pycapio.internals import *
```

This imports the native functions and classes — {func}`~pycapio.internals.pycapio_open`,
the IO wrappers, the scandir iterator, and so on — listed in {doc}`api/internals`
and described in {doc}`api/native`.
