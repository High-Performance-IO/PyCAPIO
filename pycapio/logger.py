from os import getenv
from datetime import datetime
from .global_vars import OriginalOpen
import inspect
DEBUG = getenv("DEBUG", "off").lower() == "on"

logfile_name = getenv("LOGFILE", "PyCapio.log")

def log(message):
    if DEBUG:
        caller = inspect.currentframe().f_back.f_code.co_name
        with OriginalOpen(logfile_name, "a") as f:
            message = message.strip("\x00")
            f.write(f"[{datetime.now()}] DBG {caller}: {message}\n")