from pycapio.internals import *
import pycapio

CAPIO_DIR = "/tmp"
pycapio._CAPIO_DIR = CAPIO_DIR

from pycapio import (
    mkdir_proxy,
    makedirs_proxy,
    scandir_proxy,
    open_proxy,
    listdir_proxy
)

from utils import *


def test_mkdir_proxy():
    assert not is_capio_running()
    pycapio_init(CAPIO_DIR=CAPIO_DIR, CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()

    mkdir_proxy("./test_directory")
    assert os.path.exists("./test_directory")
    os.rmdir("./test_directory")

    mkdir_proxy(f"{CAPIO_DIR}/test_directory")
    assert not os.path.exists(f"{CAPIO_DIR}/test_directory")

    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()

#def test_makedirs_proxy():
#    assert not is_capio_running()
#    pycapio_init(CAPIO_DIR=CAPIO_DIR, CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
#    assert is_capio_running()

#    makedirs_proxy(f"{CAPIO_DIR}/test_directory/test1/test2/test3")
#    assert not os.path.exists(f"{CAPIO_DIR}/test_directory/test1/test2/test3")

#    pycapio_teardown()
#    kill_capio_server()
#    assert not is_capio_running()


def test_dir_proxies():
    assert not is_capio_running()
    pycapio_init(CAPIO_DIR=CAPIO_DIR, CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()

    FILES = [
        "file1",
        "file2",
        "file3",
        "file4",
    ]

    mkdir_proxy(f"{CAPIO_DIR}/test_directory")

    for file in FILES:
        wrapper = open_proxy(f"{CAPIO_DIR}/test_directory/{file}", "w")
        wrapper.write("".join(["a" for _ in range(1024)]))
        wrapper.close()

    assert listdir_proxy(f"{CAPIO_DIR}/test_directory") == FILES

    for file in scandir_proxy(f"{CAPIO_DIR}/test_directory"):
        assert file.name in FILES
        read_wrapper =open_proxy(f"{CAPIO_DIR}/test_directory/{file.name}")
        assert len(read_wrapper.read()) == 1024
        read_wrapper.close()

    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()
