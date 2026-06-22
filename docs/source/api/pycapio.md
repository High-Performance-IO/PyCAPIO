# `pycapio` — high-level API

The top-level package exposes the {func}`~pycapio.CapioContext` decorator and the
CAPIO-aware proxy functions that replace Python's built-in I/O entry points.

## Package

```{eval-rst}
.. automodule:: pycapio
   :members:
   :undoc-members:
   :show-inheritance:
   :exclude-members: PyCapioPath, PyCapioTextIOWrapper, PyCapioBinaryIOWrapper,
       PyCapioScandirWrapper, DirEntry, FILE_MODES, pycapio_init,
       pycapio_teardown, pycapio_open, pycapio_mkdir, pycapio_get_capio_dir
```

## Command-line launcher

```{eval-rst}
.. automodule:: pycapio.__main__
   :members:
   :undoc-members:
```
