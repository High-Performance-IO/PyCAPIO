# PyCAPIO

**PyCAPIO** is a python module that allows the python interpreter to communicate directly with the CAPIO server. It
achieves that by monkeypatching winthin a context the builtins IO python primitives

## PyCAPIO module

Libcapio can also be used directly from Python via the `pycapio` module.

```bash
(venv) pycapio --capio-dir dir --app-name app --workflow-name ./workflow my_step.py
```

### CapioContext

Importing the CapioContext module from within the pycapio module, allwos for a more fine grained control over the
regions that interception should occur. Only I/O performed within the context block will be intercepted by CAPIO.

```python
from pycapio import CapioContext


@CapioContext(capio_dir=".", app_name="reader", workflow_name="example_workflow")
def read(path):
    with open(path, "r") as f:
        data = f.read()
    return data

read("example.txt")
```

## Notes
- PyCAPIO only works with libraries and codes that uses python builtin methods. If your script uses external libraries 
  compiled into .so files, please refer to the standard ```LD_PRELOAD``` approach for CAPIO.