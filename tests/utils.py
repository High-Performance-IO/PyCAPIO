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


def is_capio_running():
    for proc in psutil.process_iter(['name']):
        try:
            if "capio_server" in proc.info['name'].lower():
                return True
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            continue
    return False


def kill_capio_server():
    procs_to_kill = []
    for proc in psutil.process_iter(['name']):
        try:
            if "capio_server" in proc.info['name'].lower():
                proc.terminate()
                procs_to_kill.append(proc)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
    psutil.wait_procs(procs_to_kill)
    os.remove(f"files_location_{socket.gethostname()}.txt")
    assert check_shm_cleaned()
