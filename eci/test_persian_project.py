"""Exercise the cross-platform ECI Persian source-position projection."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    project, release = dll.eciPersianProjectUtf8, dll.eciPersianRelease
    project.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
        ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t))
    project.restype = ctypes.c_int
    release.argtypes, release.restype = (ctypes.c_void_p,), None

    def run(text, positions, is_index):
        raw = text.encode('utf8')
        source = (ctypes.c_size_t * len(positions))(*positions) if positions else None
        kinds = (ctypes.c_ubyte * len(is_index))(*is_index) if is_index else None
        target = (ctypes.c_size_t * len(positions))(*([0x1234] * len(positions))) if positions else None
        output, error = ctypes.c_void_p(), ctypes.c_size_t()
        status = project(raw, len(raw), source, kinds, len(positions),
                         ctypes.byref(output), target, ctypes.byref(error))
        try:
            value = ctypes.string_at(output).decode('ascii') if output.value else None
            return status, value, tuple(target) if target is not None else (), error.value
        finally:
            if output.value:
                release(output)

    source = 'سلام دنیا'
    first = len('سلام'.encode('utf8'))
    length = len(source.encode('utf8'))
    status, text, events, _error = run(source, (0, 2, first, first + 1, length), (1, 1, 1, 1, 1))
    assert status == 0 and text == '`[.0sA.1lam] `[.1do.0nja]'
    assert events == (0, 0, len('`[.0sA.1lam]'), len('`[.0sA.1lam] '), len(text)), events
    status, text, events, error = run('سلام', (2,), (0,))
    assert status == 3 and text is None and events == (0x1234,) and error == 2
    status, text, _events, _error = run('ناموجودمطمئن', (), ())
    assert status == 1 and text is None
    status, text, _events, _error = run('3,14', (), ())
    assert status == 2 and text is None
    status, text, events, _error = run('', (0,), (1,))
    assert status == 0 and text == '' and events == (0,)
    print('Passed native ECI Persian text, source indexes and safe rejection.')


if __name__ == '__main__':
    main()
