# Libcapio

**Libcapio** is an intermediate CAPIO library that enables applications to program against the CAPIO APIs using familiar
POSIX-like semantics. It provides both a C/C++ header-only integration and a Python binding for transparent or scoped
I/O interception.


## Features

- POSIX-style APIs with a `capio_` prefix
- Simple initialization and teardown lifecycle
- Header-only C/C++ integration
- Python bindings for global or scoped I/O interception


## C/C++ Usage (`Libcapio.hpp` execution mode)

To use Libcapio in C or C++, simply include the header and call the CAPIO-prefixed APIs.

### API Style

Libcapio mirrors standard POSIX calls, replacing the function name prefix, for example:

| POSIX API | Libcapio API    |
|-----------|-----------------|
| `open()`  | `capio_open()`  |
| `read()`  | `capio_read()`  |
| `write()` | `capio_write()` |
| `close()` | `capio_close()` |

### Initialization and Teardown

Before calling any CAPIO APIs, you **must** initialize the library, and you **must** clean it up when finished.

```cpp
#include "Libcapio.hpp"

int main() {
    libcapio_init();

    int fd = capio_open("example.txt", O_RDONLY);
    // ... use CAPIO APIs ...
    capio_close(fd);

    libcapio_teardown();
    return 0;
}
```


## Python Usage (`pycapio`)

Libcapio can also be used directly from Python via the `pycapio` module.

### Global I/O Interception

Importing `pycapio` will automatically intercept all supported I/O operations for the lifetime of the process:

```python
import pycapio

with open("example.txt", "r") as f:
    data = f.read()
```

### Scoped I/O Interception

To limit interception to a specific section of code, use the `CapioContext` context manager:

```python
from  pycapio.context import CapioContext
file_path = "/tmp/sample_streaming.dat"

@CapioContext(capio_app_name="producer", capio_dir="/tmp", capio_workflow_name="CAPIO", silent=False)
def test_write_1gb():
    with open(file_path, "w") as f:
        f.write("1" * 1024 * 1024 * 1024)
```

Only I/O performed within the context block will be intercepted by CAPIO.


## Notes

- CAPIO APIs are designed to be drop-in replacements for POSIX calls.
- Python interception affects standard file I/O and compatible libraries.
- Refer to CAPIO documentation for backend configuration and runtime behavior.
