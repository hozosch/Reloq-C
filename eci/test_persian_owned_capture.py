"""Compare the C-owned Persian frame capture with the existing low-level API."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as c
from pathlib import Path
import sys
import threading
import time

class NativeFrame(c.Structure):
    _fields_ = (('tick', c.c_int64), ('controls', c.c_float * 62), ('gain', c.c_float),
                ('formants', c.c_int32), ('head', c.c_int32), ('gender', c.c_int32),
                ('s_frication', c.c_int32), ('limit_frication', c.c_int32))


class NativeSyllable(c.Structure):
    _fields_ = (('from_', c.c_int32), ('to', c.c_int32))


class OwnedCapture(c.Structure):
    _fields_ = (('frames', c.POINTER(NativeFrame)), ('frame_count', c.c_size_t),
                ('syllables', c.POINTER(NativeSyllable)), ('syllable_count', c.c_size_t),
                ('mark_samples', c.POINTER(c.c_uint64)), ('mark_count', c.c_size_t),
                ('sample_count', c.c_uint64))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(
        c.c_int, c.c_void_p, c.c_int, c.c_int, c.c_void_p)
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    dll.eciNewEx.argtypes, dll.eciNewEx.restype = (I,), H
    dll.eciDelete.argtypes, dll.eciDelete.restype = (H,), H
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)
    dll.eciSetOutputBuffer.argtypes, dll.eciSetOutputBuffer.restype = (H, I, c.POINTER(c.c_int16)), I
    dll.eciSetParam.argtypes, dll.eciSetParam.restype = (H, I, I), I
    dll.eciGetParam.argtypes, dll.eciGetParam.restype = (H, I), I
    dll.eciSpeaking.argtypes, dll.eciSpeaking.restype = (H,), I
    dll.eciStop.argtypes, dll.eciStop.restype = (H,), I
    dll.eciAddTextUtf8.argtypes, dll.eciAddTextUtf8.restype = (H, c.c_char_p, S, I), I
    dll.eciClearInput.argtypes, dll.eciClearInput.restype = (H,), I
    speak = dll.eciPersianSpeakNativeUtf8
    speak.argtypes, speak.restype = (H, c.c_char_p, S, I, c.POINTER(S), c.POINTER(I), S, c.POINTER(S)), I
    capture = dll.eciPersianCaptureNativeUtf8
    capture.argtypes, capture.restype = (H, c.c_char_p, S, I, c.POINTER(S), S, S,
                                        c.POINTER(OwnedCapture), c.POINTER(S)), I
    release = dll.eciPersianReleaseOwnedCapture
    release.argtypes, release.restype = (c.POINTER(OwnedCapture),), None
    dll.eciPersianCaptureBegin.argtypes, dll.eciPersianCaptureBegin.restype = (S,), I
    dll.eciPersianCaptureSetControlsOnly.argtypes, dll.eciPersianCaptureSetControlsOnly.restype = (I,), I
    dll.eciPersianCaptureSampleCount.argtypes, dll.eciPersianCaptureSampleCount.restype = (), S
    dll.eciPersianCaptureEnd.argtypes, dll.eciPersianCaptureEnd.restype = (
        (c.POINTER(S), c.POINTER(I)), c.POINTER(NativeFrame))
    dll.eciPersianCaptureSyllables.argtypes, dll.eciPersianCaptureSyllables.restype = (
        (c.POINTER(S), c.POINTER(I)), c.POINTER(NativeSyllable))
    dll.eciPersianCaptureRelease.argtypes, dll.eciPersianCaptureRelease.restype = (), None

    def make_voice(rate=3, gender=0):
        voice = dll.eciNewEx(0x90000)
        assert voice and dll.eciSetParam(voice, 5, rate) >= 0
        assert dll.eciSetParam(voice, 0x1010, gender) >= 0
        assert dll.eciSetParam(voice, 0x1007, 140 if gender else 85) >= 0
        assert dll.eciSetParam(voice, 0x1008, 75 if gender else 65) >= 0
        assert dll.eciSetParam(voice, 0x100b, 0) >= 0
        return voice

    def offsets_for(raw):
        space = raw.find(b' ')
        return (0, space, len(raw)) if 0 < space < len(raw) else (0, len(raw))

    def owned(voice, raw, variant, offsets, maximum=30000):
        array = (S * len(offsets))(*offsets) if offsets else None
        result, error = OwnedCapture(), S()
        status = capture(voice, raw, len(raw), variant, array, len(offsets), maximum,
                         c.byref(result), c.byref(error))
        try:
            if status:
                assert not result.frames and not result.syllables and not result.mark_samples
                return status, error.value, None
            frames = c.string_at(result.frames, result.frame_count * c.sizeof(NativeFrame)) if result.frame_count else b''
            syllables = c.string_at(result.syllables, result.syllable_count * c.sizeof(NativeSyllable)) if result.syllable_count else b''
            marks = tuple(result.mark_samples[i] for i in range(result.mark_count))
            return status, error.value, (frames, syllables, marks, result.sample_count)
        finally:
            release(c.byref(result))
            assert not result.frames and not result.syllables and not result.mark_samples
            assert result.frame_count == result.syllable_count == result.mark_count == result.sample_count == 0

    def manual(voice, raw, variant, offsets, user_callback):
        values = (I * len(offsets))(*(i + 1 for i in range(len(offsets))))
        positions = (S * len(offsets))(*offsets)
        marks = []
        @callback_type
        def receive(_voice, message, value, _context):
            if message == 0 and value:
                return 2
            if message == 2:
                marks.append((value, dll.eciPersianCaptureSampleCount()))
            return 1
        dll.eciRegisterCallback(voice, receive, None)
        assert dll.eciPersianCaptureBegin(30000) == 0
        try:
            assert dll.eciPersianCaptureSetControlsOnly(1) == 0
            error = S()
            assert speak(voice, raw, len(raw), variant, positions, values, len(offsets), c.byref(error)) == 0
            samples = dll.eciPersianCaptureSampleCount()
            count, failed = S(), I()
            frames = dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert not failed.value and count.value and frames
            frame_data = c.string_at(frames, count.value * c.sizeof(NativeFrame))
            count, failed = S(), I()
            syllables = dll.eciPersianCaptureSyllables(c.byref(count), c.byref(failed))
            # The established planner omits syllable alignment if the
            # provider cannot report it unambiguously for this request.
            syllable_data = (c.string_at(syllables, count.value * c.sizeof(NativeSyllable))
                             if count.value and not failed.value else b'')
            assert [value for value, _ in marks] == list(range(1, len(offsets) + 1))
            return frame_data, syllable_data, tuple(value for _, value in marks), samples
        finally:
            dll.eciPersianCaptureRelease()
            dll.eciRegisterCallback(voice, user_callback, None)

    cases = (('سلام دنیا.', 0), ('salam donya.', 1), ('ژژژژژ', 0),
             ('تعجب', 0), ('سوال', 0), ('سلام NVDA دنیا؟', 0),
             ('برای راه‌اندازی رایانه باید این گزینه انتخاب شود.', 0))
    checks = 0
    for rate in range(7):
        for gender in (0, 1):
            voice = make_voice(rate, gender)
            buffer = (c.c_int16 * 2048)()
            user_audio, user_marks = [], []
            @callback_type
            def user_callback(_voice, message, value, _context):
                if message == 0:
                    user_audio.extend(buffer[:value])
                elif message == 2:
                    user_marks.append(value)
                return 1
            try:
                dll.eciRegisterCallback(voice, user_callback, None)
                assert dll.eciSetOutputBuffer(voice, len(buffer), buffer) == 1
                for text, variant in cases:
                    raw = text.encode('utf8'); offsets = offsets_for(raw)
                    reference = manual(voice, raw, variant, offsets, user_callback)
                    before = len(user_audio), len(user_marks)
                    status, error, actual = owned(voice, raw, variant, offsets)
                    assert status == 0 and error == 0 and actual == reference, (rate, gender, text, status)
                    assert (len(user_audio), len(user_marks)) == before
                    assert dll.eciGetParam(voice, 0x100b) == 0
                    assert dll.eciGetParam(voice, 0x1007) == (140 if gender else 85)
                    assert dll.eciGetParam(voice, 0x1008) == (75 if gender else 65)
                    checks += 1
                # Callback and output buffer were restored after the silent capture.
                raw = 'سلام.'.encode(); offsets = (S * 1)(len(raw)); values = (I * 1)(77); error = S()
                assert speak(voice, raw, len(raw), 0, offsets, values, 1, c.byref(error)) == 0
                assert user_audio and user_marks == [77]
            finally:
                dll.eciDelete(voice)

    voice = make_voice()
    try:
        raw = 'سلام دنیا.'.encode()
        assert owned(voice, raw, 0, (len(raw), 0))[0] == -1
        assert owned(voice, raw, 0, (len(raw) + 1,))[0] == -1
        assert owned(voice, raw, 2, ())[0] == -1
        assert owned(voice, raw, 0, (), 0)[0] == -1
        assert owned(voice, b'`vs70 hello', 0, ())[0] == 1
        assert owned(voice, raw, 0, (), 1)[0] == -3
        assert owned(voice, raw, 0, (0, len(raw)))[0] == 0
        first, second, error = OwnedCapture(), OwnedCapture(), S()
        try:
            assert capture(voice, raw, len(raw), 0, None, 0, 30000, c.byref(first), c.byref(error)) == 0
            saved = c.string_at(first.frames, first.frame_count * c.sizeof(NativeFrame))
            assert saved
            assert capture(voice, raw, len(raw), 0, None, 0, 30000, c.byref(second), c.byref(error)) == 0
            assert c.string_at(first.frames, first.frame_count * c.sizeof(NativeFrame)) == saved
            release(c.byref(second))
            assert c.string_at(first.frames, first.frame_count * c.sizeof(NativeFrame)) == saved
        finally:
            release(c.byref(first)); release(c.byref(second))
        assert dll.eciPersianCaptureBegin(4) == 0
        try:
            assert owned(voice, raw, 0, ())[0] == -1
            count, failed = S(), I()
            dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert count.value == 0 and not failed.value
        finally:
            dll.eciPersianCaptureRelease()
        assert dll.eciAddTextUtf8(voice, b'hello', 5, 0) == 1
        assert owned(voice, raw, 0, ())[0] == -1
        assert dll.eciClearInput(voice) == 1
    finally:
        dll.eciDelete(voice)

    def parallel_case(text, variant):
        voice = make_voice()
        try:
            raw = text.encode('utf8')
            status, error, result = owned(voice, raw, variant, offsets_for(raw))
            assert status == error == 0
            return result
        finally:
            dll.eciDelete(voice)
    reference = [parallel_case(text, variant) for text, variant in cases[:2]]
    with ThreadPoolExecutor(max_workers=2) as executor:
        futures = [executor.submit(parallel_case, text, variant) for text, variant in cases[:2]]
        assert [f.result() for f in futures] == reference

    # An external stop must cancel silent planning and leave the handle usable.
    voice = make_voice(); raw = ('سلام دنیا ' * 1000).encode(); result = []; worker = None
    try:
        worker = threading.Thread(target=lambda: result.append(owned(voice, raw, 0, (), 120000)), daemon=True)
        worker.start(); deadline = time.monotonic() + 10
        while worker.is_alive() and not dll.eciSpeaking(voice) and time.monotonic() < deadline:
            time.sleep(.001)
        assert dll.eciSpeaking(voice), 'capture did not enter synthesis before cancellation'
        assert dll.eciStop(voice) == 1
        worker.join(15)
        assert not worker.is_alive() and result and result[0][0] == -4, result
        assert parallel_case('سلام دنیا.', 0)
        assert owned(voice, 'سلام.'.encode(), 0, (0,))[0] == 0
    finally:
        if worker and worker.is_alive():
            dll.eciStop(voice)
            worker.join(15)
        if not worker or not worker.is_alive():
            dll.eciDelete(voice)
    print(f'Passed {checks} owned/manual Persian captures, state restoration, isolation and cancellation.')


if __name__ == '__main__':
    main()
