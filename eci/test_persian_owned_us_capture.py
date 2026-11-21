"""Compare the C-owned US pilot capture with the established low-level path."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as c
from pathlib import Path
import sys
import threading
import time

from test_persian_owned_capture import NativeFrame, NativeSyllable, OwnedCapture


class Proxy(c.Structure):
    _fields_ = (('text', c.c_void_p), ('length', c.c_size_t),
                ('word_offsets', c.POINTER(c.c_size_t)), ('word_count', c.c_size_t))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciGetParam', I, (H, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)), ('eciInsertIndex', I, (H, I)),
        ('eciSynthesize', I, (H,)), ('eciClearInput', I, (H,)),
        ('eciSpeaking', I, (H,)), ('eciStop', I, (H,)),
        ('eciPersianUsProxyUtf8', I, (H, c.c_char_p, S, c.POINTER(Proxy))),
        ('eciPersianUsProxyRelease', None, (c.POINTER(Proxy),)),
        ('eciPersianCaptureUsProxyUtf8', I, (H, c.c_char_p, S, S, c.POINTER(OwnedCapture))),
        ('eciPersianReleaseOwnedCapture', None, (c.POINTER(OwnedCapture),)),
        ('eciPersianCaptureBegin', I, (S,)), ('eciPersianCaptureSetControlsOnly', I, (I,)),
        ('eciPersianCaptureSampleCount', S, ()),
        ('eciPersianCaptureEnd', c.POINTER(NativeFrame), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCaptureSyllables', c.POINTER(NativeSyllable), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCaptureRelease', None, ()),
    )
    for name, restype, argtypes in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = restype, argtypes
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)

    def make_voice(rate=3, gender=0):
        voice = dll.eciNewEx(0x10000)
        assert voice and dll.eciSetParam(voice, 5, rate) >= 0
        assert dll.eciSetParam(voice, 0x1010, gender) >= 0
        assert dll.eciSetParam(voice, 0x1007, 140 if gender else 85) >= 0
        assert dll.eciSetParam(voice, 0x1008, 75 if gender else 65) >= 0
        assert dll.eciSetParam(voice, 0x100b, 0) >= 0
        return voice

    def owned(voice, raw, maximum=30000):
        result = OwnedCapture()
        status = dll.eciPersianCaptureUsProxyUtf8(voice, raw, len(raw), maximum, c.byref(result))
        try:
            if status:
                assert not result.frames and not result.syllables and not result.mark_samples
                return status, None
            frames = c.string_at(result.frames, result.frame_count*c.sizeof(NativeFrame)) if result.frame_count else b''
            syllables = (c.string_at(result.syllables, result.syllable_count*c.sizeof(NativeSyllable))
                         if result.syllable_count else b'')
            marks = tuple(result.mark_samples[i] for i in range(result.mark_count))
            return status, (frames, syllables, marks, result.sample_count)
        finally:
            dll.eciPersianReleaseOwnedCapture(c.byref(result))
            assert not result.frames and not result.syllables and not result.mark_samples

    def manual(voice, raw, user_callback):
        proxy = Proxy()
        assert dll.eciPersianUsProxyUtf8(voice, raw, len(raw), c.byref(proxy)) == 0
        try:
            text = c.string_at(proxy.text, proxy.length)
            offsets = [proxy.word_offsets[i] for i in range(proxy.word_count-1)]
        finally:
            dll.eciPersianUsProxyRelease(c.byref(proxy))
        marks = []

        @callback_type
        def receive(_voice, message, value, _context):
            if message == 0 and value:
                return 2
            if message == 2:
                marks.append((value, dll.eciPersianCaptureSampleCount()))
            return 1

        old_tags = dll.eciGetParam(voice, 0x100b)
        assert dll.eciSetParam(voice, 0x100b, 1) >= 0
        dll.eciRegisterCallback(voice, receive, None)
        assert dll.eciPersianCaptureBegin(30000) == 0
        try:
            assert dll.eciPersianCaptureSetControlsOnly(1) == 0
            cursor = 0
            for i, offset in enumerate(offsets):
                part = text[cursor:offset]
                assert dll.eciAddTextUtf8(voice, part, len(part), 0) == 1
                assert dll.eciInsertIndex(voice, i+1) == 1
                cursor = offset
            part = text[cursor:]
            assert dll.eciAddTextUtf8(voice, part, len(part), 0) == 1
            assert dll.eciSynthesize(voice) == 1
            samples, count, failed = dll.eciPersianCaptureSampleCount(), S(), I()
            frames = dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert not failed.value and count.value and frames
            frame_data = c.string_at(frames, count.value*c.sizeof(NativeFrame))
            count, failed = S(), I()
            syllables = dll.eciPersianCaptureSyllables(c.byref(count), c.byref(failed))
            syllable_data = (c.string_at(syllables, count.value*c.sizeof(NativeSyllable))
                             if count.value and not failed.value else b'')
            assert [value for value, _ in marks] == list(range(1, len(offsets)+1))
            return frame_data, syllable_data, tuple(value for _, value in marks), samples
        finally:
            dll.eciPersianCaptureRelease()
            assert dll.eciSetParam(voice, 0x100b, old_tags) >= 0
            dll.eciRegisterCallback(voice, user_callback, None)

    cases = (b'`[.1sefr] `[.0ket.1Ab].', b'`[.1ma]?',
             b'`[.1sefr], `[.1jek], `[.1do].',
             b'`[.0ket.1Ab] `00 `[.0rA].',
             b'`[.0ra.0hAn.0da.1zi] `[.0ra.0ya.1nE] `[.1in].')
    checks = 0
    for rate in range(7):
        for gender in (0, 1):
            voice = make_voice(rate, gender)
            buffer, audio, indexes = (c.c_int16 * 2048)(), [], []

            @callback_type
            def user_callback(_voice, message, value, _context):
                if message == 0:
                    audio.extend(buffer[:value])
                elif message == 2:
                    indexes.append(value)
                return 1

            try:
                dll.eciRegisterCallback(voice, user_callback, None)
                assert dll.eciSetOutputBuffer(voice, len(buffer), buffer) == 1
                for raw in cases:
                    reference = manual(voice, raw, user_callback)
                    status, actual = owned(voice, raw)
                    assert status == 0 and actual == reference, (rate, gender, raw, status)
                    assert not audio and not indexes
                    assert dll.eciGetParam(voice, 0x100b) == 0
                    assert dll.eciGetParam(voice, 0x1007) == (140 if gender else 85)
                    assert dll.eciGetParam(voice, 0x1008) == (75 if gender else 65)
                    checks += 1
                assert dll.eciAddTextUtf8(voice, b'Hello.', 6, 0) == 1
                assert dll.eciInsertIndex(voice, 77) == 1 and dll.eciSynthesize(voice) == 1
                assert audio and indexes == [77]
            finally:
                dll.eciDelete(voice)

    voice, wrong_voice = make_voice(), dll.eciNewEx(0x90000)
    try:
        assert owned(voice, b'no phonemes')[0] == 1
        assert owned(voice, cases[0], 0)[0] == -1
        assert owned(voice, cases[0], 1)[0] == -3
        assert owned(wrong_voice, cases[0])[0] == -1
        assert dll.eciPersianCaptureBegin(4) == 0
        try:
            assert owned(voice, cases[0])[0] == -1
        finally:
            dll.eciPersianCaptureRelease()
        assert dll.eciAddTextUtf8(voice, b'hello', 5, 0) == 1
        assert owned(voice, cases[0])[0] == -1
        assert dll.eciClearInput(voice) == 1
        assert owned(voice, cases[0])[0] == 0
    finally:
        dll.eciDelete(voice); dll.eciDelete(wrong_voice)

    def parallel_case(raw):
        voice = make_voice()
        try:
            status, result = owned(voice, raw)
            assert status == 0
            return result
        finally:
            dll.eciDelete(voice)
    reference = [parallel_case(raw) for raw in cases[:2]]
    with ThreadPoolExecutor(max_workers=2) as executor:
        futures = [executor.submit(parallel_case, raw) for raw in cases[:2]]
        assert [future.result() for future in futures] == reference

    voice = make_voice(); raw = b' '.join((b'`[.1sefr]',) * 1000) + b'.'; results = []; worker = None
    try:
        worker = threading.Thread(target=lambda: results.append(owned(voice, raw, 120000)), daemon=True)
        worker.start(); deadline = time.monotonic() + 10
        while worker.is_alive() and not dll.eciSpeaking(voice) and time.monotonic() < deadline:
            time.sleep(.001)
        assert dll.eciSpeaking(voice), 'US capture did not enter synthesis before cancellation'
        assert dll.eciStop(voice) == 1
        worker.join(15)
        assert not worker.is_alive() and results and results[0][0] == -4, results
        assert owned(voice, cases[0])[0] == 0
    finally:
        if worker and worker.is_alive():
            dll.eciStop(voice)
            worker.join(15)
        if not worker or not worker.is_alive():
            dll.eciDelete(voice)
    print(f'Passed {checks} owned/manual US pilot captures, state restoration, isolation and cancellation.')


if __name__ == '__main__':
    main()
