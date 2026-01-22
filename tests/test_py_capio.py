import os

os.environ["CAPIO_DIR"] = "/tmp"
os.environ["DEBUG"] = "on"
os.environ["CAPIO_LOG_LEVEL"] = "-1"

path = "/tmp/sample.dat"
path1 = "/tmp/sample1.dat"

first_line = "first line\n"
second_line = "second line\n"
third_line = "third line\n"

def test_write():
    # Create a temporary file
    with open(path) as tmp:
        # Write some lines
        tmp.write(first_line)
        tmp.write(second_line)
        tmp.write(third_line)


def test_read_with_size():
    with open(path, "r") as f:
        content = f.read(11)
        print("Partial read content:", repr(content))
        assert content == first_line
        content = f.read(12)
        print("Partial read content:", repr(content))
        assert content == second_line
        content = f.read(11)
        print("Partial read content:", repr(content))
        assert content == third_line

def test_read():
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
        print(f"Length of read content: {len(content)}")
        assert content == f"{first_line}{second_line}{third_line}"
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

def test_write_1gb():
    with open(path1, "wb") as f:
        f.write(b"1" * 1024 * 1024 * 1024)

def test_read_1gb():
    with open(path1, "rb") as f:
        data = f.read()
    for char in data:
        assert char == "1"