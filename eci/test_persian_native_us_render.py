"""Exercise the C-only Persian sentence renderer without NVDA or eSpeak."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as c
from pathlib import Path
import re
import sys
import threading
import time

from test_persian_owned_capture import OwnedCapture


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciGetParam', I, (H, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)), ('eciClearInput', I, (H,)),
        ('eciSpeaking', I, (H,)), ('eciStop', I, (H,)),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                         c.POINTER(I), S, S, c.POINTER(S))),
        ('eciPersianProjectTextCompleteUtf8', I, (c.c_char_p, S, I, c.POINTER(S),
                                                  c.POINTER(c.c_ubyte), S, c.POINTER(H),
                                                  c.POINTER(S), c.POINTER(S))),
        ('eciPersianRelease', None, (H,)),
        ('eciPersianCaptureNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S), S, S,
                                           c.POINTER(OwnedCapture), c.POINTER(S))),
        ('eciPersianCaptureUsProxyUtf8', I, (H, c.c_char_p, S, S, c.POINTER(OwnedCapture))),
        ('eciPersianReleaseOwnedCapture', None, (c.POINTER(OwnedCapture),)),
        ('eciPersianReplayUsCapture', I, (H, c.POINTER(OwnedCapture), c.POINTER(OwnedCapture),
                                        c.POINTER(c.c_uint32), H, S)),
    )
    for name, result, parameters in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback, H)

    class Pair:
        def __init__(self, rate=3, gender=0):
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
            for handle, buf, cb in ((self.voice, self.buffer, receive),
                                    (self.pilot, self.pilot_buffer, silent)):
                dll.eciRegisterCallback(handle, cb, None)
                assert dll.eciSetOutputBuffer(handle, len(buf), buf) == 1
                for param, value in ((5, rate), (0x1010, gender), (0x1007, 140 if gender else 85),
                                     (0x1008, 75 if gender else 65), (0x100b, 0)):
                    assert dll.eciSetParam(handle, param, value) >= 0

        def render(self, text, mode=0, offsets=(), maximum=30000):
            raw = text.encode('utf8'); self.audio.clear(); self.indices.clear()
            positions = (S * len(offsets))(*offsets) if offsets else None
            values = (I * len(offsets))(*(90 + i for i in range(len(offsets)))) if offsets else None
            error = S()
            status = dll.eciPersianSpeakUsNativeUtf8(self.voice, self.pilot, raw, len(raw), mode,
                        positions, values, len(offsets), maximum, c.byref(error))
            return status, bytes(self.audio), tuple(self.indices), error.value

        def manual(self, text, mode=0, offsets=()):
            raw = text.encode('utf8'); projected, error = H(), S()
            assert dll.eciPersianProjectTextCompleteUtf8(raw, len(raw), mode, None, None, 0,
                                                        c.byref(projected), None, c.byref(error)) == 0
            try:
                tagged = c.string_at(projected)
            finally:
                dll.eciPersianRelease(projected)
            targets, donor = OwnedCapture(), OwnedCapture()
            positions = (S * len(offsets))(*offsets) if offsets else None
            try:
                assert dll.eciPersianCaptureNativeUtf8(self.voice, raw, len(raw), mode, positions,
                        len(offsets), 30000, c.byref(targets), c.byref(error)) == 0
                assert dll.eciPersianCaptureUsProxyUtf8(self.pilot, tagged, len(tagged), 30000,
                                                       c.byref(donor)) == 0
                assert targets.mark_count == donor.mark_count
                tags = re.findall(rb'`\[([^]]+)\]', tagged)
                counts = (c.c_uint32 * len(tags))(*(len(re.findall(rb'\.[012]', tag)) for tag in tags))
                self.audio.clear(); self.indices.clear()
                assert dll.eciPersianReplayUsCapture(self.voice, c.byref(targets), c.byref(donor),
                                                    counts, None, 0) == 0
                return bytes(self.audio)
            finally:
                dll.eciPersianReleaseOwnedCapture(c.byref(targets))
                dll.eciPersianReleaseOwnedCapture(c.byref(donor))

        def close(self):
            dll.eciDelete(self.voice); dll.eciDelete(self.pilot)

    cases = (('سلام', 0), ('سلام دنیا.', 0), ('کتاب را.', 0),
             ('سلام، دنیا؟', 0), ('سلام (دنیا) خوب', 0), ('او آمد.', 0),
             ('salaam donyaa.', 1), ('salaam (donyaa) khoob', 1), ('u aamad.', 1),
             ('tajjub', 1), ('salaam aamad tajjub.', 1))
    checks = 0
    for rate in range(7):
        for gender in (0, 1):
            pair = Pair(rate, gender)
            try:
                for text, mode in cases:
                    status, audio, indices, error = pair.render(text, mode)
                    assert status == 0 and audio and not indices, (rate, gender, text, status, error)
                    raw = text.encode('utf8')
                    status, marked, indices, error = pair.render(text, mode, (0, len(raw)))
                    assert status == 0 and marked == audio, (rate, gender, text, status, error)
                    assert indices == ((90, 0), (91, len(marked) // 2)), (rate, gender, text, indices)
                    assert not pair.pilot_audio
                    assert dll.eciGetParam(pair.voice, 0x100b) == dll.eciGetParam(pair.pilot, 0x100b) == 0
                    checks += 1
                for text, mode, boundaries in (('سلام', 0, ()), ('tajjub', 1, ()),
                                               ('سلام دنیا.', 0, (len('سلام '.encode('utf8')),))):
                    actual = pair.render(text, mode)[1]
                    expected = pair.manual(text, mode, boundaries)
                    assert actual == expected, (rate, gender, text, len(actual), len(expected))
                    checks += 1
            finally:
                pair.close()

    # The native Persian transaction owns one timing choice for both its
    # target and silent US contour. A caller's independent US setting must
    # neither affect the utterance nor be overwritten after it returns.
    clock_param = 0x101b
    for rate in (1, 3, 6):
        pair = Pair(rate)
        try:
            raw = 'سلام دنیا'.encode('utf8')
            for fa_clock in (0, 1, 2):
                effective = int(fa_clock == 1)
                assert dll.eciSetParam(pair.voice, clock_param, fa_clock) >= 0
                assert dll.eciSetParam(pair.pilot, clock_param, effective) >= 0
                reference = pair.render('سلام دنیا', offsets=(0, len(raw)))
                assert reference[0] == 0 and reference[1] and len(reference[2]) == 2
                for us_clock in (0, 1, 2):
                    assert dll.eciSetParam(pair.pilot, clock_param, us_clock) >= 0
                    actual = pair.render('سلام دنیا', offsets=(0, len(raw)))
                    assert actual[:3] == reference[:3], (rate, fa_clock, us_clock)
                    assert dll.eciGetParam(pair.voice, clock_param) == fa_clock
                    assert dll.eciGetParam(pair.pilot, clock_param) == us_clock
                    # Preflight rejection and capture failure must also
                    # restore the pilot's original choice without output.
                    rejected = pair.render('سلام NVDA دنیا')
                    assert rejected[0] == 1 and not rejected[1] and not rejected[2]
                    assert dll.eciGetParam(pair.pilot, clock_param) == us_clock
                    failed = pair.render('سلام', maximum=1)
                    assert failed[0] == -3 and not failed[1] and not failed[2]
                    assert dll.eciGetParam(pair.pilot, clock_param) == us_clock
                    checks += 1
        finally:
            pair.close()

    pair = Pair()
    try:
        for text, mode, expected in (('سلام NVDA دنیا', 0, 1), ('`vs70 سلام', 0, 1), ('', 0, 1),
                                     ('سلام', 2, -1)):
            status, audio, marks, _ = pair.render(text, mode)
            assert status == expected and not audio and not marks, (text, status)
        status, audio, marks, _ = pair.render('سلام', offsets=(99,))
        assert status == -1 and not audio and not marks
        status, audio, marks, _ = pair.render('سلام', maximum=1)
        assert status == -3 and not audio and not marks
        assert dll.eciSetParam(pair.pilot, 5, 4) >= 0
        assert pair.render('سلام')[0] == -1
        assert dll.eciSetParam(pair.pilot, 5, 3) >= 0
        assert dll.eciAddTextUtf8(pair.voice, b'hello', 5, 0) == 1
        assert pair.render('سلام')[0] == -1
        assert dll.eciClearInput(pair.voice) == 1
        assert pair.render('سلام')[0] == 0
    finally:
        pair.close()

    def independent(text):
        pair = Pair()
        try:
            status, audio, marks, _ = pair.render(text, offsets=(0, len(text.encode('utf8'))))
            assert status == 0 and audio and len(marks) == 2
            return audio, marks
        finally:
            pair.close()
    samples = ('سلام دنیا.', 'کتاب را.')
    expected = [independent(text) for text in samples]
    with ThreadPoolExecutor(max_workers=2) as pool:
        assert list(pool.map(independent, samples)) == expected

    pair, result = Pair(), []
    assert dll.eciSetParam(pair.voice, clock_param, 1) >= 0
    assert dll.eciSetParam(pair.pilot, clock_param, 2) >= 0
    worker = threading.Thread(target=lambda: result.append(pair.render('سلام ' * 1000, maximum=120000)), daemon=True)
    try:
        worker.start(); deadline = time.monotonic() + 15
        while worker.is_alive() and not dll.eciSpeaking(pair.voice) and time.monotonic() < deadline:
            time.sleep(.001)
        assert dll.eciSpeaking(pair.voice), 'native Persian capture did not start'
        assert dll.eciStop(pair.voice) == 1
        worker.join(20)
        assert not worker.is_alive() and result and result[0][0] == -4, result
        assert dll.eciGetParam(pair.voice, clock_param) == 1
        assert dll.eciGetParam(pair.pilot, clock_param) == 2
        assert pair.render('سلام')[0] == 0
        assert dll.eciGetParam(pair.pilot, clock_param) == 2
    finally:
        if worker.is_alive():
            dll.eciStop(pair.voice); dll.eciStop(pair.pilot); worker.join(20)
        if not worker.is_alive():
            pair.close()
    print(f'Passed {checks} C-only Persian/Pinglish renders and low-level comparisons, with indices, isolation and cancellation.')


if __name__ == '__main__':
    main()
