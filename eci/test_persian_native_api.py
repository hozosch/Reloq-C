"""Exercise the experimental Persian API without importing an NVDA driver."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    loader = ctypes.WinDLL if sys.platform == 'win32' else ctypes.CDLL
    callback_type = ctypes.WINFUNCTYPE if sys.platform == 'win32' else ctypes.CFUNCTYPE
    dll = loader(str(args.library.resolve()))
    handle = ctypes.c_void_p
    callback_type = callback_type(ctypes.c_int, handle, ctypes.c_int, ctypes.c_int, ctypes.c_void_p)
    dll.eciNewEx.argtypes, dll.eciNewEx.restype = (ctypes.c_int,), handle
    dll.eciDelete.argtypes, dll.eciDelete.restype = (handle,), handle
    dll.eciRegisterCallback.argtypes = (handle, callback_type, ctypes.c_void_p)
    dll.eciSetOutputBuffer.argtypes, dll.eciSetOutputBuffer.restype = (
        (handle, ctypes.c_int, ctypes.POINTER(ctypes.c_int16)), ctypes.c_int)
    dll.eciSetParam.argtypes, dll.eciSetParam.restype = (handle, ctypes.c_int, ctypes.c_int), ctypes.c_int
    dll.eciGetParam.argtypes, dll.eciGetParam.restype = (handle, ctypes.c_int), ctypes.c_int
    speak = dll.eciPersianSpeakNativeUtf8
    speak.argtypes = (handle, ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                      ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_int),
                      ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t))
    speak.restype = ctypes.c_int

    for rate in (0, 3, 6):
        for gender in (0, 1):
            voice = dll.eciNewEx(0x90000)
            assert voice
            buffer = (ctypes.c_int16 * 2048)()
            audio, marks = [], []

            @callback_type
            def receive(_handle, message, value, _context):
                if message == 0:
                    if not 0 <= value <= len(buffer):
                        return 2
                    audio.extend(buffer[:value])
                elif message == 2:
                    marks.append(value)
                return 1

            try:
                dll.eciRegisterCallback(voice, receive, None)
                assert dll.eciSetOutputBuffer(voice, len(buffer), buffer) == 1
                assert dll.eciSetParam(voice, 5, rate) >= 0
                assert dll.eciSetParam(voice, 0x1010, gender) >= 0
                assert dll.eciSetParam(voice, 0x100b, 0) >= 0
                for text, variant in (('سلام دنیا.', 0), ('salam donya.', 1),
                                      ('ژژژژژ', 0), ('تعجب', 0),
                                      ('سوال', 0), ('سلام NVDA دنیا؟', 0)):
                    raw = text.encode('utf8')
                    offsets = (ctypes.c_size_t * 2)(0, len(raw))
                    values = (ctypes.c_int * 2)(11, 12)
                    error = ctypes.c_size_t()
                    audio.clear(); marks.clear()
                    assert speak(voice, raw, len(raw), variant, offsets, values, 2,
                                 ctypes.byref(error)) == 0, (text, rate, gender, error.value)
                    assert audio and max(map(abs, audio)) > 100 and marks == [11, 12], (text, marks)
                    assert dll.eciGetParam(voice, 0x100b) == 0
                for text in ('`vs70 سلام', 'سوال NVDA', '3,14'):
                    raw = text.encode('utf8'); error = ctypes.c_size_t()
                    audio.clear(); marks.clear()
                    assert speak(voice, raw, len(raw), 0, None, None, 0,
                                 ctypes.byref(error)) == 1, text
                    assert not audio and not marks
            finally:
                dll.eciDelete(voice)
    print('Passed standalone Persian ECI speech, marks, voices, rates and safe rejection.')


if __name__ == '__main__':
    main()
