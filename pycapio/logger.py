from os import getenv
from datetime import datetime
from .global_vars import OriginalOpen

DEBUG = getenv("DEBUG", "off").lower() == "on"

logfile_name = getenv("LOGFILE", "PyCapio.log")

def log(message):
    if DEBUG:
        with OriginalOpen(logfile_name, "a") as f:
            message = message.strip("\x00")
            f.write(f"[{datetime.now()}] DBG: {message}\n")