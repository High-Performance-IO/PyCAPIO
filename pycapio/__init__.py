import io
import os
from functools import wraps
from os import path
from typing import Any

import atexit
import builtins

from ._pycapio import *

__all__ = [name for name in globals() if not name.startswith("_")] + ["CapioContext"]


def _dump_context() -> dict[str, Any]:
    return {
        "builtins.open": builtins.open,
        "os.mkdir": os.mkdir,
        "os.makedirs": os.makedirs,
        "os.scandir": os.scandir,
        "io.open": io.open,
        "os.listdir": os.listdir,
        "os.path": os.path,
    }


def _restore_context(context: dict[str, Any]):
    builtins.open = context["builtins.open"]
    os.mkdir = context["os.mkdir"]
    os.makedirs = context["os.makedirs"]
    os.scandir = context["os.scandir"]
    io.open = context["io.open"]
    os.listdir = context["os.listdir"]
    os.path = context["os.path"]

def _patch_context():
    builtins.open = open_proxy
    os.mkdir = mkdir_proxy
    os.makedirs = makedirs_proxy
    os.scandir = PyCapioScandirWrapper
    io.open = open_proxy
    os.listdir = listdir_proxy
    os.path = PyCapioPath

py_capio_initialized = False

_BUILTIN_STACK = _dump_context()

_CAPIO_DIR: str | None = None


def open_proxy(*args, **kwargs):
    global _CAPIO_DIR

    arg0 = args[0] if args else None
    if not args or not isinstance(arg0, (str, bytes, os.PathLike)):
        print(f"DEBUG: Type of arg0 is {type(arg0)}")
        print(f"NOT A CAPIO PATH -> {arg0}")
        return _BUILTIN_STACK["builtins.open"](*args, **kwargs)

    target_path = _BUILTIN_STACK["os.path"].abspath(args[0])
    if _CAPIO_DIR and (not target_path.startswith(_CAPIO_DIR) or ".python_history" in target_path):
        return _BUILTIN_STACK["builtins.open"](*args, **kwargs)

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

    target_path = _BUILTIN_STACK["os.path"].abspath(path_val)
    if _CAPIO_DIR and target_path.startswith(_CAPIO_DIR):
        return pycapio_mkdir(target_path, mode)
    else:
        return _BUILTIN_STACK["os.mkdir"](path_val, mode, *args, **kwargs)


def makedirs_proxy(path_val, mode=0o777, *args, **kwargs):
    global _CAPIO_DIR

    target_path = _BUILTIN_STACK["os.path"].abspath(path_val)
    if _CAPIO_DIR and target_path.startswith(_CAPIO_DIR):
        return pycapio_mkdir(target_path, mode)
    else:
        return _BUILTIN_STACK["os.makedirs"](path_val, mode, *args, **kwargs)


def listdir_proxy(dirpath: str):
    global _CAPIO_DIR

    dirpath = _BUILTIN_STACK["os.path"].abspath(dirpath)

    if _CAPIO_DIR and dirpath.startswith(_CAPIO_DIR):
        directory_entries: list[str] = []
        for f in PyCapioScandirWrapper(dirpath):
            directory_entries.append(f.name)

        return directory_entries

    return _BUILTIN_STACK["os.listdir"](dirpath)


def CapioContext(*,
                 capio_dir=".",
                 app_name=CAPIO_DEFAULT_APP_NAME,
                 workflow_name=CAPIO_DEFAULT_WORKFLOW_NAME,
                 silent=True,
                 server_exec_path="capio_server",
                 capio_cl_configuration_file="",
                 await_server_timeout_seconds=2,
                 teardown_server=True
                 ):
    def _CapioContext(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            os.environ["SILENT"] = "ON" if silent else "OFF"

            global py_capio_initialized
            global _CAPIO_DIR
            if not py_capio_initialized:
                _CAPIO_DIR = _BUILTIN_STACK["os.path"].abspath(capio_dir)
                pycapio_init(CAPIO_DIR=_CAPIO_DIR,
                             CAPIO_WORKFLOW_NAME=workflow_name,
                             CAPIO_APP_NAME=app_name,
                             capio_server_exec_path=server_exec_path,
                             capio_cl_configuration_file=capio_cl_configuration_file,
                             await_server_timeout_seconds=await_server_timeout_seconds)
                py_capio_initialized = True
                atexit.register(pycapio_teardown, teardown_server)

            context = _dump_context()
            _patch_context()

            try:
                return func(*args, **kwargs)
            finally:
                _restore_context(context)

        return wrapper

    return _CapioContext
