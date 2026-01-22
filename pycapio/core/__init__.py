import _io
import atexit
import builtins
import io
from os import path

from .._pycapio import *

__all__ = [name for name in globals() if not name.startswith("_")] + [
    "patch",
    "unpatch",
    "OriginalOpen",
    "OriginalTextIOWrapper",
    "OriginalPythonTextIOWrapper",
    "OriginalBinaryIOWrapper",
    "OriginalPythonBinaryIOWrapper",
]

pycapio_init()

OriginalTextIOWrapper = _io.TextIOWrapper
OriginalPythonTextIOWrapper = io.TextIOWrapper
OriginalBinaryIOWrapper = _io.BufferedReader
OriginalPythonBinaryIOWrapper = io.BufferedReader
OriginalOpen = builtins.open

CAPIO_DIR = pycapio_get_capio_dir()


def open_proxy(*args, **kwargs):
    target_path = path.abspath(args[0])

    if not target_path.startswith(CAPIO_DIR.as_posix()) or ".python_history" in target_path:
        return OriginalOpen(*args, **kwargs)

    flags = kwargs.get("mode", args[1] if len(args) > 1 else "rw")

    pycapio_flags = 0
    if "w" in flags:
        pycapio_flags |= FILE_MODES["O_WRONLY"] | FILE_MODES["O_CREAT"]
    if "a" in flags:
        pycapio_flags |= FILE_MODES["O_APPEND"]
    if "r" in flags:
        pycapio_flags |= FILE_MODES["O_RDONLY"]
    if "+" in flags:
        pycapio_flags |= FILE_MODES["O_RDWR"]

    fd = pycapio_open(target_path, pycapio_flags, 777)

    return PyCapioBinaryIOWrapper(fd) if "b" in flags else PyCapioTextIOWrapper(fd)


def patch():
    io.TextIOWrapper = PyCapioTextIOWrapper
    io.BufferedReader = PyCapioBinaryIOWrapper

    _io.TextIOWrapper = PyCapioTextIOWrapper
    _io.BufferedReader = PyCapioBinaryIOWrapper

    builtins.open = open_proxy


def unpatch():
    io.TextIOWrapper = OriginalPythonTextIOWrapper
    io.BufferedReader = OriginalPythonBinaryIOWrapper

    _io.TextIOWrapper = OriginalTextIOWrapper
    _io.BufferedReader = OriginalBinaryIOWrapper

    builtins.open = OriginalOpen


atexit.register(unpatch)
patch()
