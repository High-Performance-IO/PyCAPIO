import builtins
import io
import os

import pycapio
from pycapio import CapioContext, PyCapioPath


def test_capio_context_patches_restores_and_streams():
    original_seen = {"builtins.open": builtins.open,
                     "os.mkdir": os.mkdir,
                     "os.makedirs": os.makedirs,
                     "os.scandir": os.scandir,
                     "io.open": io.open,
                     "os.listdir": os.listdir,
                     "os.path": os.path}

    seen = {}

    @CapioContext(capio_dir="/tmp", app_name="ctx", workflow_name="ctx_wf")
    def do_io():
        seen["builtins.open"] = builtins.open
        seen["os.mkdir"] = os.mkdir
        seen["os.makedirs"] = os.makedirs
        seen["os.scandir"] = os.scandir
        seen["io.open"] = io.open
        seen["os.listdir"] = os.listdir
        seen["os.path"] = os.path

        path = "/tmp/capio_context.dat"
        with open(path, "w") as f:
            f.write("streamed via CapioContext")
        with open(path, "r") as f:
            return f.read()

    result = do_io()

    assert result == "streamed via CapioContext"

    assert seen["builtins.open"] is pycapio.open_proxy
    assert seen["os.mkdir"] is pycapio.mkdir_proxy
    assert seen["os.makedirs"] is pycapio.makedirs_proxy
    assert seen["os.scandir"] is pycapio.scandir_proxy
    assert seen["io.open"] is pycapio.open_proxy
    assert seen["os.listdir"] is pycapio.listdir_proxy
    assert seen["os.path"] is PyCapioPath

    assert builtins.open is original_seen["builtins.open"]
    assert os.mkdir is original_seen["os.mkdir"]
    assert os.makedirs is original_seen["os.makedirs"]
    assert os.scandir is original_seen["os.scandir"]
    assert io.open is original_seen["io.open"]
    assert os.listdir is original_seen["os.listdir"]
    assert os.path is original_seen["os.path"]

