"""Exercise native Persian and US planning components together without NVDA."""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import sys

from test_persian_owned_capture import NativeFrame, NativeSyllable, OwnedCapture


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S, U = c.c_void_p, c.c_int, c.c_size_t, c.c_uint64
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    F, Y = c.POINTER(NativeFrame), c.POINTER(NativeSyllable)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciPersianProjectTextCompleteUtf8', I, (c.c_char_p, S, I, c.POINTER(S),
            c.POINTER(c.c_ubyte), S, c.POINTER(H), c.POINTER(S), c.POINTER(S))),
        ('eciPersianRelease', None, (H,)),
        ('eciPersianCaptureNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S), S, S,
                                            c.POINTER(OwnedCapture), c.POINTER(S))),
        ('eciPersianCaptureUsProxyUtf8', I, (H, c.c_char_p, S, S, c.POINTER(OwnedCapture))),
        ('eciPersianReleaseOwnedCapture', None, (c.POINTER(OwnedCapture),)),
        ('eciPersianTransferUsSamples', I, (F, S, F, S, c.POINTER(U), c.POINTER(U), S,
            c.c_uint, Y, S, Y, S, c.POINTER(c.c_uint32))),
        ('eciPersianReplayEpoch', c.c_uint32, (H,)),
        ('eciPersianReplayFrames', I, (H, c.c_uint32, F, S, H, S)),
    )
    for name, restype, argtypes in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = restype, argtypes
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)
    rates = (8000, 11025, 22050, 16000, 32000, 44100, 48000)
    cases = (('دو.', ()), ('سلام دنیا.', ('سلام ',)), ('کتاب را.', ('کتاب ',)))
    checks = 0
    for rate_code, rate in enumerate(rates):
        fa, en = dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)
        assert fa and en
        buffer, audio = (c.c_int16 * 2048)(), []

        @callback_type
        def receive(_handle, message, value, _context):
            if message == 0:
                audio.extend(buffer[:value])
            return 1

        try:
            for voice in (fa, en):
                assert dll.eciSetParam(voice, 5, rate_code) >= 0
            dll.eciRegisterCallback(fa, receive, None)
            assert dll.eciSetOutputBuffer(fa, len(buffer), buffer) == 1
            for text, prefixes in cases:
                raw = text.encode('utf8'); error = S(); projected = H()
                offsets = (S * len(prefixes))(*(len(x.encode('utf8')) for x in prefixes)) if prefixes else None
                target, donor = OwnedCapture(), OwnedCapture()
                try:
                    assert dll.eciPersianProjectTextCompleteUtf8(raw, len(raw), 0, None, None, 0,
                                                                 c.byref(projected), None, c.byref(error)) == 0
                    tagged = c.string_at(projected)
                    assert dll.eciPersianCaptureNativeUtf8(fa, raw, len(raw), 0, offsets, len(prefixes),
                                                           30000, c.byref(target), c.byref(error)) == 0
                    assert dll.eciPersianCaptureUsProxyUtf8(en, tagged, len(tagged), 30000, c.byref(donor)) == 0
                    assert target.mark_count == donor.mark_count == len(prefixes)
                    original = [tuple(target.frames[i].controls) for i in range(target.frame_count)]
                    tb = (U * (target.mark_count+2))(0, *target.mark_samples[:target.mark_count], target.sample_count)
                    db = (U * (donor.mark_count+2))(0, *donor.mark_samples[:donor.mark_count], donor.sample_count)
                    status = dll.eciPersianTransferUsSamples(
                        target.frames, target.frame_count, donor.frames, donor.frame_count,
                        tb, db, target.mark_count+1, rate, target.syllables, target.syllable_count,
                        donor.syllables, donor.syllable_count, None)
                    if status == 1 and target.mark_count:
                        tb, db = (U * 2)(0, target.sample_count), (U * 2)(0, donor.sample_count)
                        status = dll.eciPersianTransferUsSamples(
                            target.frames, target.frame_count, donor.frames, donor.frame_count,
                            tb, db, 1, rate, None, 0, None, 0, None)
                    assert status == 0, (rate, text, status)
                    assert any(old[1] != target.frames[i].controls[1] for i, old in enumerate(original))
                    assert all(all(old[j] == target.frames[i].controls[j] for j in range(62) if j != 1)
                               for i, old in enumerate(original))
                    audio.clear()
                    epoch = dll.eciPersianReplayEpoch(fa)
                    assert dll.eciPersianReplayFrames(fa, epoch, target.frames, target.frame_count, None, 0) == 0
                    assert audio and max(map(abs, audio)) > 100, (rate, text)
                    checks += 1
                finally:
                    dll.eciPersianRelease(projected)
                    dll.eciPersianReleaseOwnedCapture(c.byref(target))
                    dll.eciPersianReleaseOwnedCapture(c.byref(donor))
        finally:
            dll.eciDelete(fa); dll.eciDelete(en)
    print(f'Passed {checks} independent Persian/US capture, contour transfer and replay checks.')


if __name__ == '__main__':
    main()
