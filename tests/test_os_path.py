import os

from pycapio import PyCapioPath
from pycapio.internals import FILE_MODES, pycapio_open

def test_basename():
    assert PyCapioPath.basename("/tmp/a/b.txt") == "b.txt"


def test_dirname():
    assert PyCapioPath.dirname("/tmp/a/b.txt") == "/tmp/a"


def test_join_basic():
    assert PyCapioPath.join("/tmp/a", "b.txt") == "/tmp/a/b.txt"


def test_join_empty_operands():
    assert PyCapioPath.join("", "") == ""
    assert PyCapioPath.join("", "b") == "b"
    assert PyCapioPath.join("a", "") == "a"


def test_splitext_returns_stem_and_extension():
    # wrapper returns the stem, not the full root
    assert PyCapioPath.splitext("/tmp/a/b.txt") == ("b", ".txt")
    assert PyCapioPath.splitext("/tmp/a/noext") == ("noext", "")


def test_isabs():
    assert PyCapioPath.isabs("/tmp/a") is True
    assert PyCapioPath.isabs("a/b") is False


def test_normpath_collapses_dotdot():
    assert PyCapioPath.normpath("/tmp/a/../b") == "/tmp/b"


def test_split_into_head_and_tail():
    assert PyCapioPath.split("/tmp/a/b.txt") == ("/tmp/a", "b.txt")


def test_normcase_is_identity_on_unix():
    assert PyCapioPath.normcase("/Tmp/MixedCase") == "/Tmp/MixedCase"


def test_relpath():
    assert PyCapioPath.relpath("/tmp/a/b", "/tmp") == "a/b"


def test_exists_and_isfile_for_written_file(capio):
    path = f"{capio}/ospath_file.dat"

    fd = pycapio_open(path, FILE_MODES["O_CREAT"], 0o777)
    assert fd != -1
    from pycapio.internals import PyCapioTextIOWrapper

    wrapper = PyCapioTextIOWrapper(fd)
    wrapper.write("some content")
    del wrapper  # flush + close

    assert PyCapioPath.exists(path) is True
    assert PyCapioPath.isfile(path) is True
    # the file is purely virtual: it never hits the real filesystem
    assert not os.path.exists(path)