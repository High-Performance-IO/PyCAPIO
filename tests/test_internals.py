from pycapio.internals import *
from utils import *


def test_init():
    assert not is_capio_running()
    pycapio_init(CAPIO_DIR="/tmp", CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()
    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()


def test_open():
    pycapio_init(CAPIO_DIR="/tmp", CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()
    fd = pycapio_open("/tmp/test.dat", FILE_MODES["O_CREAT"], 0o777)
    assert fd != -1
    assert not os.path.exists("/tmp/test.dat")
    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()


def test_text_io_wrapper():
    pycapio_init(CAPIO_DIR="/tmp", CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()
    fd = pycapio_open("/tmp/test.dat", FILE_MODES["O_CREAT"], 0o777)
    assert fd != -1
    assert not os.path.exists("/tmp/test.dat")

    wrapper = PyCapioTextIOWrapper(fd)

    BUFFER = """
    Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas erat augue, tincidunt ut sem rhoncus, gravida 
    ]rutrum lectus. Praesent sagittis purus finibus faucibus fermentum. Suspendisse varius augue justo, in feugiat leo 
    congue non. Donec arcu velit, sagittis eget purus quis, ultricies volutpat mi. Praesent tempor dui vel mollis 
    accumsan. Vivamus nisi purus, rhoncus placerat felis a, pharetra tempor est. Etiam euismod tempor diam, sit amet 
    sodales ipsum congue eget. Morbi sit amet nisi a lorem pretium auctor congue eu dui. Vestibulum metus est, malesuada 
    id pulvinar sit amet, dignissim in sapien. Etiam maximus finibus est vitae ultrices. Etiam in vestibulum metus. 
    Phasellus efficitur nulla suscipit, tristique elit quis, vulputate sapien. Praesent interdum semper diam non 
    sollicitudin. Vivamus finibus id sapien quis pharetra. Aenean et mauris at lorem rutrum viverra."""

    assert wrapper.write(BUFFER) == len(BUFFER)

    del wrapper

    fd = pycapio_open("/tmp/test.dat", FILE_MODES["O_RDONLY"], 0o777)
    assert fd != -1
    assert not os.path.exists("/tmp/test.dat")

    wrapper_read = PyCapioTextIOWrapper(fd)
    assert BUFFER == wrapper_read.read()

    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()


def test_binary_io_wrapper():
    pycapio_init(CAPIO_DIR="/tmp", CAPIO_WORKFLOW_NAME="test", CAPIO_APP_NAME="test")
    assert is_capio_running()
    fd = pycapio_open("/tmp/test.dat", FILE_MODES["O_CREAT"], 0o777)
    assert fd != -1
    assert not os.path.exists("/tmp/test.dat")

    wrapper = PyCapioBinaryIOWrapper(fd)

    BUFFER = b"""
    Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas erat augue, tincidunt ut sem rhoncus, gravida 
    ]rutrum lectus. Praesent sagittis purus finibus faucibus fermentum. Suspendisse varius augue justo, in feugiat leo 
    congue non. Donec arcu velit, sagittis eget purus quis, ultricies volutpat mi. Praesent tempor dui vel mollis 
    accumsan. Vivamus nisi purus, rhoncus placerat felis a, pharetra tempor est. Etiam euismod tempor diam, sit amet 
    sodales ipsum congue eget. Morbi sit amet nisi a lorem pretium auctor congue eu dui. Vestibulum metus est, malesuada 
    id pulvinar sit amet, dignissim in sapien. Etiam maximus finibus est vitae ultrices. Etiam in vestibulum metus. 
    Phasellus efficitur nulla suscipit, tristique elit quis, vulputate sapien. Praesent interdum semper diam non 
    sollicitudin. Vivamus finibus id sapien quis pharetra. Aenean et mauris at lorem rutrum viverra."""

    assert wrapper.write(BUFFER) == len(BUFFER)

    del wrapper

    fd = pycapio_open("/tmp/test.dat", FILE_MODES["O_RDONLY"], 0o777)
    assert fd != -1
    assert not os.path.exists("/tmp/test.dat")

    wrapper_read = PyCapioBinaryIOWrapper(fd)
    assert BUFFER == wrapper_read.read()

    pycapio_teardown()
    kill_capio_server()
    assert not is_capio_running()
