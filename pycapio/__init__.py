"""Top-level package for PyCAPIO.

PyCAPIO brings transparent data streaming to file-based Python workflows by
*monkey patching* Python's built-in I/O entry points (:func:`open`,
:func:`os.mkdir`, :func:`os.scandir`, ...) and routing any access that targets
the configured CAPIO directory through the native CAPIO server instead of the
operating system.

The public surface of this module is intentionally small:

* :func:`CapioContext` -- a decorator that initialises CAPIO and patches the
  built-in I/O functions for the duration of the decorated call.
* the ``*_proxy`` callables (:func:`open_proxy`, :func:`mkdir_proxy`, ...) --
  drop-in replacements for the corresponding built-ins that dispatch to CAPIO
  when a path lives inside the CAPIO directory and fall back to the original
  implementation otherwise.

Everything else re-exported here (``pycapio_open``, ``PyCapioPath``,
``FILE_MODES``, the IO wrappers, ...) comes from the compiled extension
:mod:`pycapio._pycapio` and is documented in the *Native API* section.

Example:
    Intercept only the I/O performed inside a single function::

        from pycapio import CapioContext

        @CapioContext(capio_dir=".", app_name="reader",
                      workflow_name="example_workflow")
        def read(path):
            with open(path, "r") as f:
                return f.read()
"""

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
    """Capture the currently installed built-in I/O callables.

    Returns:
        A mapping from dotted built-in name (for example ``"builtins.open"``)
        to the callable currently bound to it. This snapshot is used to
        restore the originals once a :func:`CapioContext` region exits.
    """
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
    """Reinstate the original built-in I/O callables.

    Args:
        context: A snapshot previously produced by :func:`_dump_context`.
    """
    builtins.open = context["builtins.open"]
    os.mkdir = context["os.mkdir"]
    os.makedirs = context["os.makedirs"]
    os.scandir = context["os.scandir"]
    io.open = context["io.open"]
    os.listdir = context["os.listdir"]
    os.path = context["os.path"]

def _patch_context():
    """Replace the built-in I/O callables with their CAPIO-aware proxies.

    After this call, :func:`open`, :func:`os.mkdir`, :func:`os.makedirs`,
    :func:`os.scandir`, :func:`io.open` and :func:`os.listdir` route through the
    matching ``*_proxy`` function, and :data:`os.path` is swapped for the native
    :class:`PyCapioPath` implementation.
    """
    builtins.open = open_proxy
    os.mkdir = mkdir_proxy
    os.makedirs = makedirs_proxy
    os.scandir = scandir_proxy
    io.open = open_proxy
    os.listdir = listdir_proxy
    os.path = PyCapioPath

py_capio_initialized = False
"""bool: ``True`` once :func:`pycapio_init` has run in this process.

CAPIO is initialised at most once per process; the first
:func:`CapioContext` invocation flips this flag so later ones reuse the running
server.
"""

_BUILTIN_STACK = _dump_context()

_CAPIO_DIR: str | None = None


def scandir_proxy(path: str):
    """CAPIO-aware replacement for :func:`os.scandir`.

    Args:
        path: Directory to scan.

    Returns:
        A :class:`PyCapioScandirWrapper` when ``path`` resolves inside the
        active CAPIO directory, otherwise the result of the original
        :func:`os.scandir`.
    """
    global _CAPIO_DIR
    target_path = _BUILTIN_STACK["os.path"].abspath(path)
    if _CAPIO_DIR and target_path.startswith(_CAPIO_DIR):
        return PyCapioScandirWrapper(path)
    return _BUILTIN_STACK["os.scandir"](path)

def open_proxy(*args, **kwargs):
    """CAPIO-aware replacement for the built-in :func:`open`.

    The first positional argument is treated as the file path. The Python
    ``mode`` string (``"r"``, ``"w+"``, ``"ab"``, ...) is translated into the
    corresponding CAPIO open flags (see :data:`FILE_MODES`) before delegating to
    :func:`pycapio_open`.

    The original :func:`open` is used unchanged when any of the following hold:

    * the first argument is not a path-like object;
    * no CAPIO directory is active;
    * the target path lies outside the CAPIO directory;
    * the target path is the interactive ``.python_history`` file.

    Args:
        *args: Positional arguments accepted by :func:`open`; ``args[0]`` is the
            path and the optional ``args[1]`` is the mode.
        **kwargs: Keyword arguments accepted by :func:`open` (notably ``mode``).

    Returns:
        A :class:`PyCapioBinaryIOWrapper` for binary modes (``"b"``) or a
        :class:`PyCapioTextIOWrapper` for text modes when the path is handled by
        CAPIO; otherwise a standard file object from the built-in :func:`open`.
    """
    global _CAPIO_DIR

    arg0 = args[0] if args else None
    if not args or not isinstance(arg0, (str, bytes, os.PathLike)):
        return _BUILTIN_STACK["builtins.open"](*args, **kwargs)

    target_path = _BUILTIN_STACK["os.path"].abspath(args[0])
    if _CAPIO_DIR and (not target_path.startswith(_CAPIO_DIR) or ".python_history" in target_path):
        return _BUILTIN_STACK["builtins.open"](*args, **kwargs)

    flags_str = kwargs.get("mode", args[1] if len(args) > 1 else "r")

    pycapio_flags = 0
    if "+" in flags_str:
        pycapio_flags |= FILE_MODES["O_RDWR"]
        if "w" in flags_str:
            pycapio_flags |= FILE_MODES["O_CREAT"]
        if "a" in flags_str:
            pycapio_flags |= FILE_MODES["O_APPEND"]
    else:
        if "w" in flags_str: pycapio_flags |= FILE_MODES["O_WRONLY"] | FILE_MODES["O_CREAT"]
        if "a" in flags_str: pycapio_flags |= FILE_MODES["O_WRONLY"] | FILE_MODES["O_APPEND"]
        if "r" in flags_str: pycapio_flags |= FILE_MODES["O_RDONLY"]

    fd = pycapio_open(target_path, pycapio_flags, 0o777)
    return PyCapioBinaryIOWrapper(fd) if "b" in flags_str else PyCapioTextIOWrapper(fd)


def mkdir_proxy(path_val, mode=0o777, *args, **kwargs):
    """CAPIO-aware replacement for :func:`os.mkdir`.

    Args:
        path_val: Directory to create.
        mode: Permission bits applied to the new directory.
        *args: Extra positional arguments forwarded to the original
            :func:`os.mkdir` on the fallback path.
        **kwargs: Extra keyword arguments forwarded to the original
            :func:`os.mkdir` on the fallback path.

    Returns:
        The result of :func:`pycapio_mkdir` when ``path_val`` is inside the
        active CAPIO directory, otherwise the result of the original
        :func:`os.mkdir`.
    """
    global _CAPIO_DIR

    target_path = _BUILTIN_STACK["os.path"].abspath(path_val)
    if _CAPIO_DIR and target_path.startswith(_CAPIO_DIR):
        return pycapio_mkdir(target_path, mode)
    else:
        return _BUILTIN_STACK["os.mkdir"](path_val, mode, *args, **kwargs)


def makedirs_proxy(path_val, mode=0o777, *args, **kwargs):
    """CAPIO-aware replacement for :func:`os.makedirs`.

    Note:
        Inside the CAPIO directory the creation is delegated to
        :func:`pycapio_mkdir`; intermediate directories are handled by the CAPIO
        server rather than being created recursively in Python.

    Args:
        path_val: Directory path to create.
        mode: Permission bits applied to the new directory.
        *args: Extra positional arguments forwarded to the original
            :func:`os.makedirs` on the fallback path.
        **kwargs: Extra keyword arguments forwarded to the original
            :func:`os.makedirs` on the fallback path.

    Returns:
        The result of :func:`pycapio_mkdir` when ``path_val`` is inside the
        active CAPIO directory, otherwise the result of the original
        :func:`os.makedirs`.
    """
    global _CAPIO_DIR

    target_path = _BUILTIN_STACK["os.path"].abspath(path_val)
    if _CAPIO_DIR and target_path.startswith(_CAPIO_DIR):
        return pycapio_mkdir(target_path, mode)
    else:
        return _BUILTIN_STACK["os.makedirs"](path_val, mode, *args, **kwargs)


def listdir_proxy(dirpath: str):
    """CAPIO-aware replacement for :func:`os.listdir`.

    Args:
        dirpath: Directory whose entries should be listed.

    Returns:
        A list of entry names. Inside the CAPIO directory the names are gathered
        by iterating a :class:`PyCapioScandirWrapper`; outside it the original
        :func:`os.listdir` is used.
    """
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
    """Decorator factory that runs a function with CAPIO interception enabled.

    On first use within a process this initialises CAPIO (starting/attaching to
    the CAPIO server) and registers teardown via :mod:`atexit`. For every call
    of the decorated function it swaps the built-in I/O callables for their
    CAPIO-aware proxies, runs the function, and restores the originals
    afterwards -- so interception is scoped to the decorated call only.

    All arguments are keyword-only.

    Args:
        capio_dir: Root directory managed by CAPIO. I/O on paths *inside* this
            directory is intercepted; everything else falls back to the standard
            library. Defaults to ``"."``.
        app_name: Logical application name reported to CAPIO. Defaults to
            :data:`CAPIO_DEFAULT_APP_NAME`.
        workflow_name: Logical workflow name reported to CAPIO. Defaults to
            :data:`CAPIO_DEFAULT_WORKFLOW_NAME`.
        silent: When ``True`` (default) sets the ``SILENT`` environment variable
            to ``"ON"`` to suppress CAPIO server chatter.
        server_exec_path: Path or name of the CAPIO server executable. Defaults
            to ``"capio_server"``.
        capio_cl_configuration_file: Optional path to a CAPIO-CL configuration
            file. Empty string means no configuration file.
        await_server_timeout_seconds: How long to wait for the CAPIO server to
            become ready before giving up. Defaults to ``2``.
        teardown_server: When ``True`` (default) the CAPIO server is torn down at
            process exit.

    Returns:
        A decorator that wraps the target function with CAPIO setup,
        I/O patching and cleanup.

    Example:
        ::

            @CapioContext(capio_dir="./data", app_name="writer")
            def produce(path, payload):
                with open(path, "w") as f:
                    f.write(payload)
    """
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
