import os
from functools import wraps


def CapioContext(capio_dir=".", capio_app_name="default_app", capio_workflow_name="CAPIO"):
    def _CapioContext(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            os.environ["CAPIO_DIR"] = capio_dir
            os.environ["CAPIO_WORKFLOW_NAME"] = capio_workflow_name
            os.environ["CAPIO_APP_NAME"] = capio_app_name
            import pycapio.core
            _ret = func(*args, **kwargs)
            pycapio.core.pycapio_teardown()
            pycapio.core.unpatch()
            return _ret

        return wrapper

    return _CapioContext


__all__ = [name for name in globals() if not name.startswith("_")] + ["CapioContext"]
