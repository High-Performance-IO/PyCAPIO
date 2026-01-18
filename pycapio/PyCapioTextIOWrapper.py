from io import UnsupportedOperation

from ._pycapio import *
from .logger import log


class PyCapioTextIOWrapper:
    def __init__(self, fd: int, path: str = None, chunk_size: int = 4096):
        self.fd = fd
        self.path = path
        self.closed = False
        self._rbuf = bytearray()
        self.chunk_size = chunk_size

    def read(self, size=-1):

        if size == 0:
            return b''

        out = bytearray()


        while True:
            if self._rbuf:
                if size < 0:
                    out += self._rbuf
                    self._rbuf.clear()
                else:
                    take = min(size - len(out), len(self._rbuf))
                    out += self._rbuf[:take]
                    del self._rbuf[:take]

                if size >= 0 and len(out) == size:
                    break
            chunk = bytearray(self.chunk_size)
            n = pycapio_read(self.fd, chunk, self.chunk_size)

            if n <= 0:
                break

            self._rbuf += chunk[:n]

        return out


    def readline(self, limit=-1):
        if limit == 0:
            return b''

        out = bytearray()

        while limit < 0 or len(out) < limit:
            # Consume buffered data first
            if self._rbuf:
                new_line = self._rbuf.find(b'\n')

                take = new_line + 1 if new_line != -1 else len(self._rbuf)

                if limit > 0:
                    take = min(take, limit - len(out))

                out += self._rbuf[:take]
                del self._rbuf[:take]

                if out.endswith(b'\n'):
                    break

                continue

            # Read next chunk
            tmp = bytearray(self.chunk_size)
            n = pycapio_read(self.fd, tmp, self.chunk_size)

            if n <= 0:
                break

            self._rbuf += tmp[:n]

        return out

    def readlines(self, hint=-1):
        lines = []

        if hint >= 0:
            total = 0
            while total < hint:
                line = self.readline()
                if not line:
                    break
                lines.append(line)
                total += len(line)
        else:
            while True:
                line = self.readline()
                if not line:
                    break
                lines.append(line)

        return lines

    def readinto(self, b):
        pass

    def write(self, s):
        if isinstance(s, str):
            s = s.encode("utf-8")
        elif not isinstance(s, (bytes, bytearray, memoryview)):
            raise TypeError(f"write() argument must be str or bytes, got {type(s)}")

        return pycapio_write(self.fd, s, len(s))

    def writelines(self, lines):
        for line in lines:
            self.write(s=line)

    def seek(self, offset, whence=0):
        raise NotImplemented("Unsupported operation: seek()")

    def tell(self):
        raise NotImplemented("Unsupported operation: tell()")

    def truncate(self, size=None):
        raise NotImplemented("truncate() for PY-CAPIO module is not supported")

    def flush(self):
        raise NotImplemented("flush() for PY-CAPIO module is not supported")

    def close(self):
        pycapio_close(self.fd)
        log(f"Closed file descriptor: {self.fd}")
        self.closed = True

    def fileno(self):
        return self.fd

    def isatty(self):
        raise NotImplemented("isatty() for PY-CAPIO module is not supported")
        return False

    def readable(self):
        raise NotImplemented("readable() for PY-CAPIO module is not supported")
        return True

    def writable(self):
        raise NotImplemented("writable() for PY-CAPIO module is not supported")
        return True

    def seekable(self):
        raise NotImplemented("seekable() for PY-CAPIO module is not supported")
        return False

    def detach(self):
        raise NotImplemented("detach() for PY-CAPIO module is not supported")

    # TODO: Understand how to, and if need to handle these
    def _process_read(self, data):
        raise NotImplemented("read from stdin is not supported")

    def _process_write(self, data):
        raise NotImplemented("write to stdout is not supported")

    def __iter__(self):
        raise UnsupportedOperation("__iter__ is not supported")

    def __next__(self):
        raise UnsupportedOperation("__next__ is not supported")

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        if not self.closed:
            self.close()

    def __getattr__(self, name):
        raise UnsupportedOperation(f"{name} is not supported")
