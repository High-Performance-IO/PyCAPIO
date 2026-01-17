import _io
import builtins
import io
from os import path
from .PyCapioTextIOWrapper import PyCapioTextIOWrapper
from ._pycapio import *
from .logger import log

__all__ = [name for name in globals() if not name.startswith("_")] + ["patch",
                                                                      "unpatch",
                                                                      "OriginalTextIOWrapper",
                                                                      "OriginalPythonTextIOWrapper",
                                                                      "OriginalOpen", ]

pycapio_init()
OriginalTextIOWrapper = _io.TextIOWrapper
OriginalPythonTextIOWrapper = io.TextIOWrapper
OriginalOpen = builtins.open
CAPIO_DIR = pycapio_get_capio_dir()

def open_proxy(*args, **kwargs):
    target_path = path.abspath(args[0])

    if ".python_history" in target_path:
        return OriginalOpen(*args, **kwargs)

    if target_path.startswith(CAPIO_DIR.as_posix()):
        # Copy args since args is a tuple
        log(f"Opening path with CAPIO: {target_path}")

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
        log(f"Opened file descriptor: {fd}")
        return PyCapioTextIOWrapper(fd, target_path)

    return OriginalOpen(*args, **kwargs)


def patch():
    io.TextIOWrapper = PyCapioTextIOWrapper
    _io.TextIOWrapper = PyCapioTextIOWrapper
    builtins.open = open_proxy


def unpatch():
    io.TextIOWrapper = OriginalPythonTextIOWrapper
    _io.TextIOWrapper = OriginalTextIOWrapper
    builtins.open = OriginalOpen


patch()
