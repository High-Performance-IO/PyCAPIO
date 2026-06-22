"""Low-level CAPIO primitives re-exported from the native extension.

This subpackage exposes the raw building blocks that the high-level
:mod:`pycapio` proxies are built on top of. Import it when you need to write
custom I/O modules that talk to the CAPIO server directly::

    from pycapio.internals import *

All names below are defined in the compiled extension
:mod:`pycapio._pycapio`; this module simply curates the subset that is safe and
useful for downstream code, and is documented in detail under *Native API*.

Exported names:

``CAPIO_DEFAULT_APP_NAME``
    Default application name used when none is supplied.
``CAPIO_DEFAULT_WORKFLOW_NAME``
    Default workflow name used when none is supplied.
``pycapio_get_capio_dir``
    Return the active CAPIO directory.
``pycapio_init``
    Initialise CAPIO and start/attach to the server.
``pycapio_mkdir``
    Create a directory inside the CAPIO directory.
``pycapio_open``
    Open a CAPIO-managed file and return its descriptor.
``pycapio_teardown``
    Tear down CAPIO (optionally stopping the server).
``DirEntry``
    A single entry yielded while scanning a CAPIO directory.
``PyCapioBinaryIOWrapper``
    File-like wrapper for binary CAPIO streams.
``PyCapioScandirWrapper``
    Iterator over the contents of a CAPIO directory.
``PyCapioTextIOWrapper``
    File-like wrapper for text CAPIO streams.
``FILE_MODES``
    Mapping of POSIX open-flag names to their integer values.
"""

from .._pycapio import (
    CAPIO_DEFAULT_APP_NAME,
    CAPIO_DEFAULT_WORKFLOW_NAME,
    pycapio_get_capio_dir,
    pycapio_init,
    pycapio_mkdir,
    pycapio_open,
    pycapio_teardown,
    DirEntry,
    PyCapioBinaryIOWrapper,
    PyCapioScandirWrapper,
    PyCapioTextIOWrapper,
    FILE_MODES
)

__all__ = [
    "CAPIO_DEFAULT_APP_NAME",
    "CAPIO_DEFAULT_WORKFLOW_NAME",
    "pycapio_get_capio_dir",
    "pycapio_init",
    "pycapio_mkdir",
    "pycapio_open",
    "pycapio_teardown",
    "DirEntry",
    "PyCapioBinaryIOWrapper",
    "PyCapioScandirWrapper",
    "PyCapioTextIOWrapper",
    "FILE_MODES"
]
