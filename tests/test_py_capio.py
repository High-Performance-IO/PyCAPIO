import os

os.environ["CAPIO_DIR"] = "/tmp"
os.environ["DEBUG"] = "on"

import pycapio

path = "/tmp/sample.dat"

def test_write():
    # Create a temporary file
    with open(path) as tmp:
        # Write some lines
        tmp.write("first line\n")
        tmp.write("second line\n")
        tmp.write("third line\n")
        tmp.close()  # close the file


def test_read():
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
        assert content == "first line\nsecond line\nthird line\n"
        print("Full read content:", repr(content))


def test_readlines():
    with open(path, "r", encoding="utf-8") as f:
        lines = []
        while True:
            line = f.readline()
            if not line:
                break
            lines.append(line)

        expected_lines = ["first line\n", "second line\n", "third line\n"]
        assert lines == expected_lines

