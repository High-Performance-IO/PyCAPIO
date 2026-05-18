import os

from pycapio.internals import *
from utils import *

DIRECTORY_NAME = "/tmp/test_directory"

FILES_TO_CHECK = [
    "test_file_1.txt",
    "test_file_2.txt",
    "test_file_3.txt",
    "test_file_4.txt",
]


def test_create_directory():
    assert not is_capio_running()
    pycapio_init(CAPIO_DIR="/tmp", CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()
    pycapio_mkdir(DIRECTORY_NAME)
    assert not os.path.exists(DIRECTORY_NAME)
    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()

def test_create_files_in_directory():
    assert not is_capio_running()
    pycapio_init(CAPIO_DIR="/tmp", CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()
    pycapio_mkdir(DIRECTORY_NAME)
    for file in FILES_TO_CHECK:
        pycapio_open(os.path.join(DIRECTORY_NAME, file), FILE_MODES["O_CREAT"], 0o777)
        assert not os.path.exists(os.path.join(DIRECTORY_NAME, file))

    iterator = PyCapioScandirWrapper(DIRECTORY_NAME)

    for file in iterator:
        assert file.name in FILES_TO_CHECK

    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()