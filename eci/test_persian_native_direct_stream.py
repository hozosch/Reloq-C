"""Check the experimental incremental Persian C route and its boundaries."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as c
from pathlib import Path
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
        ('eciSetParam', I, (H, I, I)), ('eciGetParam', I, (H, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciSpeaking', I, (H,)), ('eciStop', I, (H,)),
        ('eciPersianSpeakNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S),
                                          c.POINTER(I), S, c.POINTER(S))),
        ('eciPersianSpeakNativeRunsUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S),
            c.POINTER(B), S, c.POINTER(S), c.POINTER(I), S, c.POINTER(S))),
    )
    for name, result, parameters in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback, H)

    class Voice:
        def __init__(self, rate=3, gender=0):
            self.handle = dll.eciNewEx(0x90000)
            assert self.handle
            self.buffer = (c.c_int16 * 2048)()
            self.audio, self.indices, self.events = bytearray(), [], []
            self.cancel, self.stop_result = False, None

            @callback
            def receive(handle, message, value, _context):
                if message == 0 and value:
                    self.audio.extend(c.string_at(self.buffer, value * 2))
                    self.events.append(('audio', dll.eciSpeaking(handle)))
                    if self.cancel:
                        self.cancel = False
                        self.stop_result = dll.eciStop(handle)
                elif message == 2:
                    self.indices.append((value, len(self.audio) // 2))
                    self.events.append(('index', value))
                return 1

            self.receive = receive
            dll.eciRegisterCallback(self.handle, receive, None)
            assert dll.eciSetOutputBuffer(self.handle, len(self.buffer), self.buffer) == 1
            for param, value in ((5, rate), (0x1010, gender), (0x1007, 140 if gender else 85),
                                 (0x1008, 75 if gender else 65), (0x100b, 0)):
                assert dll.eciSetParam(self.handle, param, value) >= 0

        def render(self, text, variant=0, runs=None, inside=(), cancel=False):
            raw = text.encode('utf8')
            self.audio.clear(); self.indices.clear(); self.events.clear()
            self.cancel, self.stop_result = cancel, None
            locations = (0, *inside, len(raw))
            offsets = (S * len(locations))(*locations)
            values = (I * len(locations))(*(71 + i for i in range(len(locations))))
            error = S()
            if runs is None:
                status = dll.eciPersianSpeakNativeUtf8(self.handle, raw, len(raw), variant,
                    offsets, values, len(locations), c.byref(error))
            else:
                ends = (S * len(runs))(*(end for end, _ in runs)) if runs else None
                modes = (B * len(runs))(*(mode for _, mode in runs)) if runs else None
                status = dll.eciPersianSpeakNativeRunsUtf8(self.handle, raw, len(raw), variant,
                    ends, modes, len(runs), offsets, values, len(locations), c.byref(error))
            return status, bytes(self.audio), tuple(self.indices), error.value

        def close(self):
            dll.eciDelete(self.handle)

    cases = ((0, 'سلام'), (0, 'سلام دنیا.'), (0, 'سلام، دنیا؟'), (0, 'او آمد.'),
             (0, 'تعجب'), (0, 'سوال'), (0, 'سلام (دنیا) خوب'), (0, '۱۲'),
             (0, 'سلام NVDA دنیا'), (0, 'سلام Windows'),
             (1, 'salaam'), (1, 'salaam donyaa.'), (1, 'u aamad.'), (1, 'tajjub'),
             (1, 'salaam (donyaa) khoob'), (1, 'salaam NVDA donyaa'), (1, '12'))
    checks = 0
    for rate in range(7):
        for gender in (0, 1):
            voice = Voice(rate, gender)
            try:
                for variant, text in cases:
                    raw = text.encode('utf8')
                    actual = voice.render(text, variant, ((len(raw), 0),))
                    expected = voice.render(text, variant)
                    assert actual == expected and actual[0] == 0 and actual[1], (rate, gender, text, actual[0])
                    assert actual[2] == ((71, 0), (72, len(actual[1]) // 2)), (text, actual[2])
                    checks += 1
            finally:
                voice.close()

    voice = Voice()
    try:
        for variant, text, prefix in ((0, 'سلام دنیا.', 'سلام '),
                                       (0, 'تعجب سلام', 'تعجب '),
                                       (1, 'salaam donyaa.', 'salaam ')):
            raw = text.encode('utf8'); boundary = len(prefix.encode('utf8'))
            actual = voice.render(text, variant, ((boundary, 0), (len(raw), 0)), inside=(boundary,))
            expected = voice.render(text, variant, inside=(boundary,))
            assert actual == expected and actual[0] == 0, (text, actual[0], expected[0])
            checks += 1

        switched, plain = 'سلام `l20.1 salaam `l20.0 دنیا', 'سلام سلام دنیا'
        actual = voice.render(switched, 0, ((len(switched.encode()), 0),))
        expected = voice.render(plain)
        assert actual == expected and actual[0] == 0, (actual[0], actual[3])
        checks += 1

        for char, tag in (('ب', '`[.1be]'), ('َ', '`[.1A]'), ('\x7f', '`[.1hAzf]')):
            actual = voice.render(char, 0, ((len(char.encode()), 1),))
            expected = voice.render(tag, 0, ((len(tag.encode()), 2),))
            assert actual == expected and actual[0] == 0 and actual[1], (char, actual[0], expected[0])
            checks += 1

        source, tagged = 'سلام ب دنیا', 'سلام `[.1be] دنیا'
        runs = ((len('سلام '.encode()), 0), (len('سلام ب'.encode()), 1),
                (len(source.encode()), 0))
        actual = voice.render(source, 0, runs)
        expected = voice.render(tagged, 0, ((len(tagged.encode()), 0),))
        assert actual == expected and actual[0] == 0, (actual[0], actual[3], expected[0])
        checks += 1

        for text, run_mode in (('⚭', 1), ('a', 1), ('`vs70 سلام', 0), ('`vb65 سلام', 0),
                               ('`ts1 سلام', 0), ('`p100 سلام', 0), ('`vwpm120 سلام', 0),
                               ('`[.1salam]', 0), ('`[.1salam]', 2)):
            status, audio, marks, error = voice.render(text, 0, ((len(text.encode()), run_mode),))
            assert status == 1 and not audio and not marks, (text, run_mode, status, error)
            checks += 1
        for text in ('`vs70 سلام', '`[.1salam]'):
            status, audio, marks, error = voice.render(text, 0, ((1, 0), (len(text.encode()), 0)))
            assert status == 1 and not audio and not marks, (text, status, error)
            checks += 1
        assert voice.render('سلام', 0, ((1, 0),))[0] == -1
        assert voice.render('سلام', 0, ((len('سلام'.encode()), 3),))[0] == -1
        assert voice.render('سلام', 0, ())[0] == -1
        status, audio, marks, _ = voice.render('سلام', 0, ((len('سلام'.encode()), 0),), inside=(99,))
        assert status == -1 and not audio and not marks
        status, audio, marks, _ = voice.render('سلام', 2, ((len('سلام'.encode()), 0),))
        assert status == -1 and not audio and not marks

        text = 'سلام دنیا. ' * 20
        status, audio, marks, error = voice.render(text, 0, ((len(text.encode()), 0),))
        assert status == 0 and audio and marks
        assert ('audio', 1) in voice.events
        assert voice.events.index(('audio', 1)) < voice.events.index(('index', 72))
        status, audio, marks, error = voice.render(text, 0, ((len(text.encode()), 0),), cancel=True)
        assert status == -4 and voice.stop_result == 1 and audio and all(value != 72 for value, _ in marks), (
            status, voice.stop_result, len(audio), marks)
        assert dll.eciGetParam(voice.handle, 0x100b) == 0
        status, audio, marks, _ = voice.render('سلام', 0, ((len('سلام'.encode()), 0),))
        assert status == 0 and audio and len(marks) == 2
        assert dll.eciGetParam(voice.handle, 0x100b) == 0
        checks += 3
    finally:
        voice.close()

    def independent(variant, text):
        own = Voice()
        try:
            result = own.render(text, variant, ((len(text.encode()), 0),))
            return result[0] == 0 and bool(result[1]) and len(result[2]) == 2
        finally:
            own.close()

    with ThreadPoolExecutor(max_workers=2) as pool:
        assert all(pool.map(lambda item: independent(*item), ((0, 'سلام دنیا.'), (1, 'salaam donyaa.'))))
    checks += 2
    print(f'Passed {checks} incremental Persian/Pinglish renders, boundaries and cancellation checks.')


if __name__ == '__main__':
    main()
