"""Compare the owned English worker with the previous mixed C route."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as c
from pathlib import Path
import sys
import tempfile


def library(path):
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(path.resolve()))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    for name, result, parameters in (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciStop', I, (H,)), ('eciSetDictionaryPath', I, (H, c.c_char_p)),
        ('eciPersianPrepareUsPilot', I, (H,)),
        ('eciPersianSpeakNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S),
                                          c.POINTER(I), S, c.POINTER(S))),
        ('eciPersianSpeakNativeRunsUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S),
            c.POINTER(c.c_ubyte), S, c.POINTER(S), c.POINTER(I), S, c.POINTER(S))),
    ):
        function = getattr(dll, name); function.restype, function.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback, H)
    return dll, callback


class Voice:
    def __init__(self, path):
        self.dll, callback = library(path)
        self.handle = self.dll.eciNewEx(0x90000)
        assert self.handle
        self.buffer = (c.c_int16 * 2048)()
        self.audio, self.marks = bytearray(), []
        self.cancel_at = None
        self.cancel_mark = None
        self.cancel_armed = False

        @callback
        def receive(handle, message, value, _context):
            if message == 0 and value:
                self.audio.extend(c.string_at(self.buffer, value * 2))
                if self.cancel_armed or (self.cancel_at is not None and len(self.audio) >= self.cancel_at):
                    self.cancel_at = None
                    self.cancel_armed = False
                    assert self.dll.eciStop(handle) == 1
            elif message == 2:
                self.marks.append((value, len(self.audio)))
                if value == self.cancel_mark:
                    self.cancel_armed = True
            return 1

        self.receive = receive
        self.dll.eciRegisterCallback(self.handle, receive, None)
        assert self.dll.eciSetOutputBuffer(self.handle, len(self.buffer), self.buffer) == 1
        assert self.dll.eciSetParam(self.handle, 0x100b, 0) >= 0
        assert self.dll.eciPersianPrepareUsPilot(self.handle) == 0

    def configure(self, rate, gender, clock, speed=65):
        for key, value in ((5, rate), (0x1010, gender), (0x1007, speed), (0x101b, clock)):
            assert self.dll.eciSetParam(self.handle, key, value) >= 0

    def dictionary(self, directory):
        path = str(directory).encode('utf8') if directory else None
        assert self.dll.eciSetDictionaryPath(self.handle, path) == 1

    def render(self, text, interior=(), cancel_at=None, cancel_mark=None):
        raw = text.encode('utf8'); self.audio.clear(); self.marks.clear(); self.cancel_at = cancel_at
        self.cancel_mark = cancel_mark; self.cancel_armed = False
        positions = (0, *interior, len(raw))
        offsets = (c.c_size_t * len(positions))(*positions)
        values = (c.c_int * len(positions))(*(71 + i for i in range(len(positions))))
        error = c.c_size_t()
        ends = (c.c_size_t * 1)(len(raw))
        modes = (c.c_ubyte * 1)(0)
        status = self.dll.eciPersianSpeakNativeRunsUtf8(self.handle, raw, len(raw), 0,
            ends, modes, 1, offsets, values, len(positions), c.byref(error))
        return status, bytes(self.audio), tuple(self.marks), error.value

    def close(self):
        self.dll.eciDelete(self.handle)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    previous, current = Voice(args.previous), Voice(args.library)
    cases = ('سلام NVDA دنیا', 'سلام Windows ۱۲ کتاب', 'Hello سلام world دنیا',
             'خوب they’ve stopped آن', 'سلام computer دنیا Windows', 'تعجب NVDA دنیا')
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for clock in (0, 1, 2):
                    for voice in (previous, current):
                        voice.configure(rate, gender, clock)
                    for text in (*cases, *reversed(cases)):
                        raw = text.encode('utf8')
                        pivot = raw.find(b'NVDA')
                        inside = (pivot + 2,) if pivot >= 0 else ()
                        a, b = previous.render(text, inside), current.render(text, inside)
                        assert a == b, (text, rate, gender, clock, a[0], b[0], a[2], b[2])
                        if 'تعجب' not in text:
                            assert a[0] == 0 and a[1], (text, a[0], a[3])
                        checks += 1
        for speed in (40, 82, 120, 65):
            for voice in (previous, current):
                voice.configure(3, 0, 0, speed)
            assert previous.render(cases[0]) == current.render(cases[0]), speed
        for voice in (previous, current):
            voice.configure(3, 0, 0)
            aborted = voice.render('سلام NVDA دنیا hello دیگر', cancel_at=5000)
            assert aborted[0] == -4 and aborted[1] and all(value != 72 for value, _ in aborted[2]), aborted[:1]
            offset = len('سلام '.encode('utf8'))
            aborted = voice.render('سلام NVDA دنیا hello دیگر', interior=(offset,), cancel_mark=72)
            assert aborted[0] == -4 and any(value == 72 for value, _ in aborted[2]) and all(
                value != 73 for value, _ in aborted[2]), aborted[:1]
        assert previous.render(cases[0]) == current.render(cases[0])
        default = current.render(cases[0])
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp)
            (folder / 'ENUmain.dic').write_bytes(b'NVDA\tbanana\n')
            for voice in (previous, current):
                voice.dictionary(folder)
            selected = current.render(cases[0])
            assert previous.render(cases[0]) == selected and selected[1] != default[1]
            for voice in (previous, current):
                voice.dictionary(None)
            assert previous.render(cases[0]) == current.render(cases[0]) == default
    finally:
        previous.close(); current.close()
    def independently(text):
        voice = Voice(args.library)
        try:
            voice.configure(3, 0, 0)
            first = voice.render(text)
            return first[0] == 0 and bool(first[1]) and all(
                voice.render(text) == first for _ in range(8))
        finally:
            voice.close()
    with ThreadPoolExecutor(max_workers=2) as pool:
        assert all(pool.map(independently, (cases[0], cases[2])))
    print(f'Passed {checks} mixed C worker, settings, cancellation and dictionary comparisons.')


if __name__ == '__main__':
    main()
