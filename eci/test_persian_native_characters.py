"""Exercise standalone character names in the independent Persian C path."""
from __future__ import annotations

import argparse
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
    declarations = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciGetParam', I, (H, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)),
        ('eciInsertIndex', I, (H, I)), ('eciSynthesize', I, (H,)), ('eciStop', I, (H,)),
        ('eciPersianCharacterResolve', I, (c.c_uint32, c.c_char_p, S, c.POINTER(I))),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                           c.POINTER(I), S, S, c.POINTER(S))),
        ('eciPersianSpeakUsNativeRunsUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                               c.POINTER(B), S, c.POINTER(S),
                                               c.POINTER(I), S, S, c.POINTER(S))),
    )
    for name, result, parameters in declarations:
        fn = getattr(dll, name)
        fn.restype, fn.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback, H)

    class Engine:
        def __init__(self, language, rate, gender):
            self.handle = dll.eciNewEx(language)
            assert self.handle
            self.buffer = (c.c_int16 * 2048)()
            self.audio, self.marks, self.stop_next = bytearray(), [], False
            @callback
            def receive(handle, message, value, _context):
                assert handle == self.handle
                if message == 0 and value:
                    self.audio.extend(c.string_at(self.buffer, value * 2))
                    if self.stop_next:
                        self.stop_next = False
                        assert dll.eciStop(self.handle) == 1
                elif message == 2:
                    self.marks.append((value, len(self.audio) // 2))
                return 1
            self.callback = receive
            dll.eciRegisterCallback(self.handle, receive, None)
            assert dll.eciSetOutputBuffer(self.handle, len(self.buffer), self.buffer) == 1
            for parameter, value in ((5, rate), (0x1010, gender), (0x1007, 85), (0x100b, 0)):
                assert dll.eciSetParam(self.handle, parameter, value) >= 0

        def clear(self):
            self.audio.clear(); self.marks.clear()

        def close(self):
            dll.eciDelete(self.handle)

    checks = 0
    for rate in (1, 3, 6):
        for gender in (0, 1):
            fa, pilot, english = (Engine(language, rate, gender)
                                  for language in (0x90000, 0x10000, 0x10000))
            try:
                def render(char, variant, run):
                    raw = char.encode('utf8'); fa.clear(); pilot.clear()
                    offsets = (S * 3)(0, len(raw) - 1 if len(raw) > 1 else 0, len(raw))
                    values = (I * 3)(71, 72, 73); error = S()
                    if run is None:
                        status = dll.eciPersianSpeakUsNativeUtf8(
                            fa.handle, pilot.handle, raw, len(raw), variant,
                            offsets, values, 3, 30000, c.byref(error))
                    else:
                        end, kind = S(len(raw)), B(run)
                        status = dll.eciPersianSpeakUsNativeRunsUtf8(
                            fa.handle, pilot.handle, raw, len(raw), variant,
                            c.byref(end), c.byref(kind), 1, offsets, values, 3,
                            30000, c.byref(error))
                    assert status == 0 and fa.audio and not pilot.audio, (char, variant, run, status, error.value)
                    assert fa.marks == [(71, 0), (72, 0), (73, len(fa.audio) // 2)], (char, run, fa.marks)
                    assert dll.eciGetParam(fa.handle, 0x100b) == 0
                    return bytes(fa.audio), tuple(fa.marks)

                for variant in (0, 1):
                    for char in (':', '.', '،', 'ب', 'َ', '\x7f', '\x1c', '✓', ' ', '🙂'):
                        assert render(char, variant, None) == render(char, variant, 0) == render(char, variant, 1)
                        checks += 1
                    if variant == 1:
                        assert render('a', variant, None) == render('a', variant, 1)
                        checks += 1
                    for char in ('⚭', 'Ж'):
                        raw = char.encode('utf8'); name = c.create_string_buffer(2048); kind = I()
                        assert dll.eciPersianCharacterResolve(ord(char), name, len(name), c.byref(kind)) == 1 and kind.value == 1
                        expected = render(char, variant, None)
                        assert expected == render(char, variant, 0) == render(char, variant, 1)
                        english.clear()
                        assert dll.eciInsertIndex(english.handle, 71) == 1
                        assert dll.eciInsertIndex(english.handle, 72) == 1
                        assert dll.eciAddTextUtf8(english.handle, name.value, len(name.value), 0) == 1
                        assert dll.eciInsertIndex(english.handle, 73) == 1
                        assert dll.eciSynthesize(english.handle) == 1
                        assert (bytes(english.audio), tuple(english.marks)) == expected, char
                        checks += 1
                    def prose(text, symbol, spelling=False):
                        raw = text.encode('utf8'); token = symbol.encode('utf8')
                        before = len(left.encode('utf8')); after = before + len(token)
                        assert raw[before:after] == token
                        offsets = (S * 4)(0, before, after, len(raw))
                        values = (I * 4)(81, 82, 83, 84); error = S()
                        fa.clear(); pilot.clear()
                        if spelling:
                            ends = (S * 3)(before, after, len(raw))
                            modes = (B * 3)(0, 1, 0)
                            status = dll.eciPersianSpeakUsNativeRunsUtf8(
                                fa.handle, pilot.handle, raw, len(raw), variant,
                                ends, modes, 3, offsets, values, 4, 30000, c.byref(error))
                        else:
                            status = dll.eciPersianSpeakUsNativeUtf8(
                                fa.handle, pilot.handle, raw, len(raw), variant,
                                offsets, values, 4, 30000, c.byref(error))
                        assert status == 0 and fa.audio and not pilot.audio, (text, status, error.value)
                        return bytes(fa.audio), tuple(fa.marks)
                    left, right = ('سلام ', ' دنیا') if variant == 0 else ('salam ', ' donya')
                    for symbol in ('⚭', '✓', '∑', 'Ж', '\x7f', '\x1c'):
                        normal = left + symbol + right
                        blank = left + ' ' * len(symbol.encode('utf8')) + right
                        # Compare the same original byte offsets and number of
                        # replacement spaces, not just total audio duration.
                        assert prose(normal, symbol) == prose(blank, ' ' * len(symbol.encode('utf8'))), (variant, symbol)
                        checks += 1
                    normal = left + '✓' + right
                    assert prose(normal, '✓', spelling=True)[0] != prose(normal, '✓')[0]
                    checks += 1
                    emoji = left + '🙂' + right
                    blank = left + ' ' * len('🙂'.encode('utf8')) + right
                    assert prose(emoji, '🙂')[0] != prose(blank, ' ' * len('🙂'.encode('utf8')))[0]
                    checks += 1
                    # A host language boundary may hand over just one symbol
                    # from a longer source. Bit 4 retains that context while
                    # explicit spelling and emoji continue to be named.
                    def context(char, mode):
                        raw = char.encode('utf8'); fa.clear(); pilot.clear()
                        end, kind = S(len(raw)), B(mode)
                        offsets, values, error = (S * 2)(0, len(raw)), (I * 2)(91, 92), S()
                        status = dll.eciPersianSpeakUsNativeRunsUtf8(
                            fa.handle, pilot.handle, raw, len(raw), variant,
                            c.byref(end), c.byref(kind), 1, offsets, values, 2,
                            30000, c.byref(error))
                        assert not pilot.audio
                        return status, bytes(fa.audio), tuple(fa.marks)
                    for char in (':', '⚭', '✓', 'Ж', '\x7f', '\x1c'):
                        assert context(char, 4) == (0, b'', ((91, 0), (92, 0))), char
                        named = context(char, 5)
                        assert named[0] == 0 and named[1] and named[2][0] == (91, 0), char
                        checks += 1
                    for mode in (4, 5):
                        named = context('🙂', mode)
                        assert named[0] == 0 and named[1] and named[2][0] == (91, 0)
                        checks += 1
                    assert context('⚭', 7)[0] == -1
                    assert context('⚭', 8)[0] == -1
                # Latin letters in a Persian profile keep their US-English
                # character-name route; Pinglish retains its Persian names.
                actual = render('a', 0, None)
                english.clear()
                assert dll.eciInsertIndex(english.handle, 71) == 1
                assert dll.eciInsertIndex(english.handle, 72) == 1
                assert dll.eciAddTextUtf8(english.handle, b'a', 1, 1) == 1
                assert dll.eciInsertIndex(english.handle, 73) == 1
                assert dll.eciSynthesize(english.handle) == 1
                assert (bytes(english.audio), tuple(english.marks)) == actual
                checks += 1
                # Cancellation is attributed to the user's Persian handle,
                # even when an English Unicode name is being rendered.
                fa.clear(); fa.stop_next = True; error = S(); raw = '⚭'.encode('utf8')
                status = dll.eciPersianSpeakUsNativeUtf8(
                    fa.handle, pilot.handle, raw, len(raw), 0,
                    None, None, 0, 30000, c.byref(error))
                assert status == -4 and fa.audio and not fa.stop_next, (status, len(fa.audio))
                assert render('⚭', 0, None)[0]
                checks += 1
            finally:
                fa.close(); pilot.close(); english.close()
    print(f'Passed {checks} standalone Persian, Pinglish and English character-name comparisons.')


if __name__ == '__main__':
    main()
