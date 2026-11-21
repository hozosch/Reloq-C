"""Exercise the native US-contour planner without the NVDA adapter."""
from __future__ import annotations
import argparse
import ctypes as c
from pathlib import Path
import sys

class Result(c.Structure):
    _fields_ = [('text', c.c_void_p), ('length', c.c_size_t),
                ('offsets', c.POINTER(c.c_size_t)), ('words', c.c_size_t)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    loader = c.WinDLL if sys.platform == 'win32' else c.CDLL
    cbtype = c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE
    dll = loader(str(args.library.resolve()))
    H, I = c.c_void_p, c.c_int
    callback = cbtype(I, H, I, I, H)
    for name, result, parameters in (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciGetParam', I, (H, I)),
        ('eciAddTextUtf8', I, (H, c.c_char_p, c.c_size_t, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_short))),
        ('eciSynthesize', I, (H,)),
        ('eciPersianUsProxyUtf8', I, (H, c.c_char_p, c.c_size_t, c.POINTER(Result))),
        ('eciPersianUsProxyRelease', None, (c.POINTER(Result),)),
    ):
        fn = getattr(dll, name); fn.argtypes = parameters; fn.restype = result
    dll.eciRegisterCallback.argtypes = (H, callback, H)
    voice, non_us = dll.eciNewEx(0x10000), dll.eciNewEx(0x90000)
    assert voice and non_us
    samples, audio = (c.c_short * 2048)(), bytearray()

    @callback
    def on_audio(_handle, message, count, _data):
        if message == 0:
            assert 0 <= count <= len(samples)
            audio.extend(c.string_at(samples, count * 2))
        return 1

    def add(text):
        raw = text.encode('utf-8')
        assert dll.eciAddTextUtf8(voice, raw, len(raw), 0)

    def render(text):
        audio.clear(); add(text); assert dll.eciSynthesize(voice)
        return bytes(audio)

    try:
        dll.eciRegisterCallback(voice, on_audio, None)
        assert dll.eciSetOutputBuffer(voice, len(samples), samples)
        assert dll.eciSetParam(voice, 0x100b, 0) >= 0
        assert dll.eciSetParam(voice, 0, 0) >= 0
        original = render('The English worker is still working.')
        raw = b'`[.1sefr] `[.0ket.1Ab].'
        output = Result()
        assert dll.eciPersianUsProxyUtf8(voice, raw, len(raw), c.byref(output)) == 0
        try:
            assert c.string_at(output.text, output.length) == b'`[.1sEfs] `[.0kEt.1Ab].'
            assert output.words == 2
            assert output.offsets[0] == len(b'`[.1sEfs] ')
        finally:
            dll.eciPersianUsProxyRelease(c.byref(output))
        assert dll.eciGetParam(voice, 0x100b) == 0
        assert dll.eciGetParam(voice, 0) == 0
        assert render('The English worker is still working.') == original

        # Never consume queued user input or borrow a non-US language handle.
        add('The English worker is still working.')
        assert dll.eciPersianUsProxyUtf8(voice, raw, len(raw), c.byref(output)) == 1
        assert not output.text and not output.words
        audio.clear(); assert dll.eciSynthesize(voice)
        assert bytes(audio) == original
        assert dll.eciPersianUsProxyUtf8(non_us, raw, len(raw), c.byref(output)) == 1
        assert not output.text and not output.words
        assert dll.eciPersianUsProxyUtf8(voice, b'no phonemes', 11, c.byref(output)) == 1
        assert not output.text and not output.words
        invalid = b'`[.1a]\xff'
        assert dll.eciPersianUsProxyUtf8(voice, invalid, len(invalid), c.byref(output)) == 1
        assert not output.text and not output.words
        assert render('The English worker is still working.') == original
        print('Native US contour API: callback, queue, voice state and invalid input passed.')
    finally:
        dll.eciDelete(voice)
        dll.eciDelete(non_us)

if __name__ == '__main__':
    main()
