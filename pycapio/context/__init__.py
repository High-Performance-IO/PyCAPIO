import atexit
import os
import builtins
from functools import wraps
from os import path

from .._pycapio import *

__all__ = ["CapioContext"]

py_capio_initialized = False

_REAL_OPEN = builtins.open
_REAL_MKDIR = os.mkdir
_REAL_MAKEDIRS = os.makedirs
_REAL_SCANDIR = os.scandir

_CAPIO_DIR = None


def open_proxy(*args, **kwargs):
    global _CAPIO_DIR

    if not args or not isinstance(args[0], (str, bytes, os.PathLike)):
        return _REAL_OPEN(*args, **kwargs)

    target_path = path.abspath(args[0])
    if not target_path.startswith(_CAPIO_DIR) or ".python_history" in target_path:
        return _REAL_OPEN(*args, **kwargs)

    flags_str = kwargs.get("mode", args[1] if len(args) > 1 else "r")

    pycapio_flags = 0
    if "w" in flags_str: pycapio_flags |= FILE_MODES["O_WRONLY"] | FILE_MODES["O_CREAT"]
    if "a" in flags_str: pycapio_flags |= FILE_MODES["O_APPEND"]
    if "r" in flags_str: pycapio_flags |= FILE_MODES["O_RDONLY"]
    if "+" in flags_str: pycapio_flags |= FILE_MODES["O_RDWR"]

    fd = pycapio_open(target_path, pycapio_flags, 0o777)
    return PyCapioBinaryIOWrapper(fd) if "b" in flags_str else PyCapioTextIOWrapper(fd)


def mkdir_proxy(path_val, mode=0o777, *args, **kwargs):
    global _CAPIO_DIR

    target_path = path.abspath(path_val)
    if target_path.startswith(_CAPIO_DIR):
        return pycapio_mkdir(target_path, mode)
    else:
        return _REAL_MKDIR(path_val, mode, *args, **kwargs)


def makedirs_proxy(path_val, mode=0o777, *args, **kwargs):
    global _CAPIO_DIR

    target_path = path.abspath(path_val)
    if target_path.startswith(_CAPIO_DIR):
        return pycapio_mkdir(target_path, mode)
    else:
        return _REAL_MAKEDIRS(path_val, mode, *args, **kwargs)


def CapioContext(*,
                 dir=".",
                 app_name=CAPIO_DEFAULT_APP_NAME,
                 workflow_name=CAPIO_DEFAULT_WORKFLOW_NAME,
                 silent=True,
                 server_exec_path="capio_server",
                 capio_cl_configuration_file="",
                 await_server_timeout_seconds=2):
    def _CapioContext(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            os.environ["SILENT"] = "ON" if silent else "OFF"

            global py_capio_initialized
            global _CAPIO_DIR
            if not py_capio_initialized:
                _CAPIO_DIR = path.abspath(dir)
                pycapio_init(CAPIO_DIR=_CAPIO_DIR,
                             CAPIO_WORKFLOW_NAME=workflow_name,
                             CAPIO_APP_NAME=app_name,
                             capio_server_exec_path=server_exec_path,
                             capio_cl_configuration_file=capio_cl_configuration_file,
                             await_server_timeout_seconds=await_server_timeout_seconds)
                py_capio_initialized = True
                atexit.register(pycapio_teardown)

            builtins.open = open_proxy
            os.mkdir = mkdir_proxy
            os.makedirs = makedirs_proxy
            os.scandir = PyCapioScandirWrapper

            try:
                return func(*args, **kwargs)
            finally:
                builtins.open = _REAL_OPEN
                os.mkdir = _REAL_MKDIR
                os.makedirs = _REAL_MAKEDIRS
                os.scandir = _REAL_SCANDIR

        return wrapper

    return _CapioContext
