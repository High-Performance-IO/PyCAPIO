def CapioContext(func):
    def _manager(*args, **kwargs):
        import pycapio.core
        _ret = func(*args, **kwargs)
        pycapio.core.pycapio_teardown()
        pycapio.core.unpatch()
        return _ret

    return _manager


__all__ = [name for name in globals() if not name.startswith("_")] + ["CapioContext"]
