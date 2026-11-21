"""Exercise independently rendered Persian text, spelling and phonetics."""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import random
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S, B = c.c_void_p, c.c_int, c.c_size_t, c.c_ubyte
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                         c.POINTER(I), S, S, c.POINTER(S))),
        ('eciPersianSpeakUsNativeRunsUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
            c.POINTER(B), S, c.POINTER(S), c.POINTER(I), S, S, c.POINTER(S))),
    )
    for name, result, parameters in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback, H)

    class Pair:
        def __init__(self, rate, gender):
            self.voice, self.pilot = dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)
            assert self.voice and self.pilot
            self.buffer, self.pilot_buffer = (c.c_int16 * 2048)(), (c.c_int16 * 2048)()
            self.audio, self.indices, self.pilot_audio = bytearray(), [], []

            @callback
            def receive(_handle, message, value, _context):
                if message == 0 and value:
                    self.audio.extend(c.string_at(self.buffer, value * 2))
                elif message == 2:
                    self.indices.append((value, len(self.audio) // 2))
                return 1

            @callback
            def silent(_handle, message, value, _context):
                if message == 0 and value:
                    self.pilot_audio.append(value)
                return 1

            self.receive, self.silent = receive, silent
            for handle, buffer, cb in ((self.voice, self.buffer, receive),
                                       (self.pilot, self.pilot_buffer, silent)):
                dll.eciRegisterCallback(handle, cb, None)
                assert dll.eciSetOutputBuffer(handle, len(buffer), buffer) == 1
                for param, value in ((5, rate), (0x1010, gender), (0x1007, 140 if gender else 85),
                                     (0x1008, 75 if gender else 65), (0x100b, 0)):
                    assert dll.eciSetParam(handle, param, value) >= 0

        def render(self, text, variant, runs=None, marks=True, inside=()):
            raw = text.encode('utf8'); self.audio.clear(); self.indices.clear()
            locations = (0, *inside, len(raw)) if marks else ()
            offsets = (S * len(locations))(*locations) if locations else None
            values = (I * len(locations))(*(71 + i for i in range(len(locations)))) if locations else None
            error = S()
            if runs is None:
                status = dll.eciPersianSpeakUsNativeUtf8(self.voice, self.pilot, raw, len(raw), variant,
                    offsets, values, len(locations), 30000, c.byref(error))
            else:
                ends = (S * len(runs))(*(end for end, _ in runs)) if runs else None
                modes = (B * len(runs))(*(mode for _, mode in runs)) if runs else None
                status = dll.eciPersianSpeakUsNativeRunsUtf8(self.voice, self.pilot, raw, len(raw), variant,
                    ends, modes, len(runs), offsets, values, len(locations), 30000, c.byref(error))
            return status, bytes(self.audio), tuple(self.indices), error.value

        def close(self):
            dll.eciDelete(self.voice); dll.eciDelete(self.pilot)

    base = ((0, 'سلام'), (0, 'سلام دنیا.'), (0, 'سلام، دنیا؟'), (0, 'او آمد.'),
            (0, 'تعجب'), (0, 'سوال'), (0, 'سلام (دنیا) خوب'), (0, '۱۲'),
            (1, 'salaam'), (1, 'salaam donyaa.'), (1, 'u aamad.'), (1, 'tajjub'),
            (1, 'salaam (donyaa) khoob'), (1, '12'))
    checks = 0
    for rate in range(7):
        for gender in (0, 1):
            pair = Pair(rate, gender)
            try:
                for mode, text in base:
                    raw = text.encode('utf8')
                    actual = pair.render(text, mode, ((len(raw), 0),))
                    expected = pair.render(text, mode)
                    assert actual == expected and actual[0] == 0 and actual[1], (rate, gender, text, actual[:1])
                    assert actual[2] == ((71, 0), (72, len(actual[1]) // 2))
                    checks += 1
                assert not pair.pilot_audio
            finally:
                pair.close()

    rng = random.Random(31415)
    fa = ('سلام', 'دنیا', 'کتاب', 'را', 'من', 'او', 'آمد', 'تعجب', 'سوال', 'خوب', 'ژژژژژ')
    pinglish = ('salaam', 'donyaa', 'ketaab', 'raa', 'man', 'u', 'aamad', 'tajjub', 'soal', 'khoob')
    pair = Pair(3, 0)
    try:
        for mode, words in ((0, fa), (1, pinglish)):
            for _ in range(80):
                text = ' '.join(rng.choice(words) for _ in range(rng.randrange(1, 6))) + rng.choice(('', '.', '?'))
                raw = text.encode('utf8')
                actual = pair.render(text, mode, ((len(raw), 0),))
                expected = pair.render(text, mode)
                assert actual == expected and actual[0] == 0, (mode, text, actual[0], expected[0])
                checks += 1
        for mode, text, prefix in ((0, 'سلام دنیا.', 'سلام '),
                                   (0, 'تعجب سلام', 'تعجب '),
                                   (1, 'salaam donyaa.', 'salaam '),
                                   (1, 'tajjub aamad.', 'tajjub ')):
            raw = text.encode('utf8'); internal = (len(prefix.encode('utf8')),)
            actual = pair.render(text, mode, ((len(raw), 0),), inside=internal)
            expected = pair.render(text, mode, inside=internal)
            assert actual == expected and actual[0] == 0, (
                mode, text, actual[0], expected[0], actual[2], expected[2], len(actual[1]), len(expected[1]))
            checks += 1
        switched = 'سلام `l20.1 salaam `l20.0 دنیا'
        plain = 'سلام سلام دنیا'
        result = pair.render(switched, 0, ((len(switched.encode()), 0),))
        expected = pair.render(plain, 0)
        assert result == expected and result[0] == 0, (result[0], result[3])
        checks += 1
        # Character names and explicit native phonetic groups are rendered
        # with the same selected contour, alone and inside ordinary text.
        for char, tag in (('ب', '`[.1be]'), ('َ', '`[.1A]'), ('\x7f', '`[.1hAzf]')):
            a = pair.render(char, 0, ((len(char.encode()), 1),))
            b = pair.render(tag, 0, ((len(tag.encode()), 2),))
            assert a == b and a[0] == 0 and a[1], (char, a[0], b[0])
            checks += 1
        source, tagged = 'سلام ب دنیا', 'سلام `[.1be] دنیا'
        boundaries = ((len('سلام '.encode()), 0), (len('سلام ب'.encode()), 1),
                      (len(source.encode()), 0))
        a = pair.render(source, 0, boundaries)
        b = pair.render(tagged, 0, ((len(tagged.encode()), 0),))
        assert a == b and a[0] == 0, (a[0], a[3], b[0], b[3])
        checks += 1
        for text, mode in (('سلام NVDA دنیا', 0), ('⚭', 1), ('a', 1), ('`vs70 سلام', 0)):
            run_mode = 1 if len(text) == 1 else 0
            status, audio, marks, error = pair.render(text, mode, ((len(text.encode()), run_mode),))
            if text == 'a':
                assert status == 0 and audio and marks, (text, status, error)
            else:
                assert status == 1 and not audio and not marks, (text, status, error)
            checks += 1
        assert pair.render('سلام', 0, ((1, 0),))[0] == -1
        assert pair.render('سلام', 0, ((len('سلام'.encode()), 3),))[0] == -1
        assert pair.render('سلام', 0, ())[0] == -1
        unsafe = '`[.1salam]'
        for mode in (0, 2):
            status, audio, marks, error = pair.render(unsafe, 0, ((len(unsafe), mode),))
            assert status == 1 and not audio and not marks, (mode, status, error)
    finally:
        pair.close()
    print(f'Passed {checks} independent Persian sentence, input-variant and rejection comparisons.')


if __name__ == '__main__':
    main()
