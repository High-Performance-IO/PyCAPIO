import os
import socket
from pathlib import Path

import psutil


def check_shm_cleaned():
    for file_path in Path("/dev/shm").rglob("*"):
        if file_path.is_file():
            try:
                if "CAPIO" in file_path.read_text(errors='ignore'):
                    return False
            except (PermissionError, FileNotFoundError):
                continue

    return True


def _is_capio_proc(proc):
    try:
        name = (proc.info.get("name") or "").lower()
        if "capio_server" in name:
            return True
        cmdline = proc.info.get("cmdline") or []
        return any("capio_server" in part.lower() for part in cmdline)
    except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
        return False


def is_capio_running():
    for proc in psutil.process_iter(["name", "cmdline"]):
        if _is_capio_proc(proc):
            return True
    return False


def kill_capio_server():
    procs_to_kill = []
    for proc in psutil.process_iter(["name", "cmdline"]):
        if _is_capio_proc(proc):
            try:
                proc.terminate()
                procs_to_kill.append(proc)
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
    psutil.wait_procs(procs_to_kill)

    registry = f"files_location_{socket.gethostname()}.txt"
    if os.path.exists(registry):  # guard: original crashed when absent
        os.remove(registry)

    assert check_shm_cleaned()