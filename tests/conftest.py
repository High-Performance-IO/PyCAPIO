import os
import socket

import psutil
import pytest

from utils import is_capio_running


def _terminate_capio_servers():
    killed = []
    for proc in psutil.process_iter(["name", "cmdline"]):
        try:
            name = (proc.info.get("name") or "").lower()
            cmdline = " ".join(proc.info.get("cmdline") or []).lower()
            if "capio_server" in name or "capio_server" in cmdline:
                proc.terminate()
                killed.append(proc)
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            continue
    if killed:
        psutil.wait_procs(killed, timeout=10)


def _remove_files_location():
    registry = f"files_location_{socket.gethostname()}.txt"
    if os.path.exists(registry):
        try:
            os.remove(registry)
        except OSError:
            pass


def force_cleanup():
    import atexit

    import pycapio
    from pycapio.internals import pycapio_teardown

    try:
        pycapio_teardown(True)
    except Exception:
        pass

    try:
        atexit.unregister(pycapio_teardown)
    except Exception:
        pass
    pycapio.py_capio_initialized = False

    _terminate_capio_servers()
    _remove_files_location()


@pytest.fixture
def capio():
    from pycapio.internals import pycapio_init

    capio_dir = "/tmp"

    assert not is_capio_running()
    pycapio_init(
        CAPIO_DIR=capio_dir,
        CAPIO_WORKFLOW_NAME="test",
        CAPIO_APP_NAME="test",
    )
    assert is_capio_running()

    try:
        yield capio_dir
    finally:
        force_cleanup()
        assert not is_capio_running()


@pytest.fixture(autouse=True)
def _ensure_clean_capio():
    yield
    force_cleanup()