import os
from functools import wraps

from .._pycapio import CAPIO_DEFAULT_APP_NAME, CAPIO_DEFAULT_WORKFLOW_NAME


def CapioContext(capio_dir=".",
                 capio_app_name=CAPIO_DEFAULT_APP_NAME,
                 capio_workflow_name=CAPIO_DEFAULT_WORKFLOW_NAME,
                 silent=True):
    def _CapioContext(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            os.environ["CAPIO_DIR"] = capio_dir
            os.environ["CAPIO_WORKFLOW_NAME"] = capio_workflow_name
            os.environ["CAPIO_APP_NAME"] = capio_app_name
            os.environ["SILENT"] = "ON" if silent else "OFF"

            import pycapio.core
            _ret = func(*args, **kwargs)
            pycapio.core.pycapio_teardown()
            pycapio.core.unpatch()
            return _ret

        return wrapper

    return _CapioContext


__all__ = [name for name in globals() if not name.startswith("_")] + ["CapioContext"]
