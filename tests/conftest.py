import os
import socket

import pytest

from utils import is_capio_running, terminate_capio_servers

# Exported by libcapio_init only when unset: clear them between tests, otherwise the workflow
# and app names of a test leak into the following ones.
_CAPIO_ENV_VARS = ("CAPIO_DIR", "CAPIO_WORKFLOW_NAME", "CAPIO_APP_NAME")


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

    terminate_capio_servers(timeout=10)
    _remove_files_location()

    for var in _CAPIO_ENV_VARS:
        os.environ.pop(var, None)


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