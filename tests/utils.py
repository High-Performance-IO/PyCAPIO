import os
import socket
from pathlib import Path

import psutil

# Workflow names used by the test suite; only shared memory segments of these workflows are
# inspected, so that other CAPIO workflows running on the same machine do not affect the tests.
TEST_WORKFLOWS = ("test", "ctx_wf")


def _is_test_workflow_segment(name: str) -> bool:
    name = name.removeprefix("sem.")
    return any(name == wf or name.startswith(wf + "_") for wf in TEST_WORKFLOWS)


def check_shm_cleaned():
    for file_path in Path("/dev/shm").iterdir():
        if not file_path.is_file() or not _is_test_workflow_segment(file_path.name):
            continue
        try:
            if "CAPIO" in file_path.read_text(errors='ignore'):
                return False
        except (PermissionError, FileNotFoundError):
            continue

    return True


def _is_capio_proc(proc):
    try:
        if "capio_server" in proc.name().lower():
            return True
        return any("capio_server" in part.lower() for part in proc.cmdline())
    except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
        return False


def capio_servers():
    """Return the live capio_server processes started by this test process.

    PyCAPIO forks the server from the process calling pycapio_init, so the servers booted by the
    tests are descendants of the test process. Servers belonging to other users, sessions or
    workflows on the same machine are ignored, and are never terminated by the cleanup helpers.
    Zombies (servers that already exited but were not reaped yet) are not running.
    """
    servers = []
    for proc in psutil.Process().children(recursive=True):
        try:
            if proc.status() == psutil.STATUS_ZOMBIE:
                continue
        except psutil.NoSuchProcess:
            continue
        if _is_capio_proc(proc):
            servers.append(proc)
    return servers


def is_capio_running():
    return len(capio_servers()) > 0


def terminate_capio_servers(timeout=None):
    procs_to_kill = []
    for proc in capio_servers():
        try:
            proc.terminate()
            procs_to_kill.append(proc)
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
    # wait_procs also reaps the servers, which are children of this process
    psutil.wait_procs(procs_to_kill, timeout=timeout)
    return procs_to_kill


def kill_capio_server():
    terminate_capio_servers()

    registry = f"files_location_{socket.gethostname()}.txt"
    if os.path.exists(registry):  # guard: original crashed when absent
        os.remove(registry)

    assert check_shm_cleaned()
