"""Compare an owned US pilot with the established explicit-pilot C API."""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import sys
import threading
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(parser.parse_args().library.resolve()))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)), ('eciGetParam', I, (H, I)),
        ('eciSetVoiceParam', I, (H, I, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciRegisterCallback', None, (H, callback, H)),
        ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)), ('eciClearInput', I, (H,)),
        ('eciSpeaking', I, (H,)), ('eciStop', I, (H,)),
        ('eciPersianPrepareUsPilot', I, (H,)),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                         c.POINTER(I), S, S, c.POINTER(S))),
        ('eciPersianSpeakUsNativeRunsUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
            c.POINTER(c.c_ubyte), S, c.POINTER(S), c.POINTER(I), S, S, c.POINTER(S))),
    )
    for name, result, parameters in functions:
        fn = getattr(dll, name)
        fn.restype, fn.argtypes = result, parameters
    voice, pilot = dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)
    assert voice and pilot
    output = (c.c_int16 * 2048)()
    audio, marks = bytearray(), []

    @callback
    def receive(_handle, message, value, _context):
        if message == 0 and value:
            audio.extend(c.string_at(output, value * 2))
        elif message == 2:
            marks.append((value, len(audio)))
        return 1

    dll.eciRegisterCallback(voice, receive, None)
    assert dll.eciSetOutputBuffer(voice, len(output), output) == 1

    def render(text, mode, external, maximum=30000, runs=None):
        raw = text.encode('utf8')
        audio.clear(); marks.clear()
        positions, values = (S * 2)(0, len(raw)), (I * 2)(7, 8)
        error = S()
        if runs is None:
            status = dll.eciPersianSpeakUsNativeUtf8(voice, pilot if external else None,
                raw, len(raw), mode, positions, values, 2, maximum, c.byref(error))
        else:
            ends, modes = (S * len(runs))(*(end for end, _ in runs)), (c.c_ubyte * len(runs))(*(kind for _, kind in runs))
            status = dll.eciPersianSpeakUsNativeRunsUtf8(voice, pilot if external else None,
                raw, len(raw), mode, ends, modes, len(runs), positions, values, 2, maximum, c.byref(error))
        return status, bytes(audio), tuple(marks), error.value

    checks = 0
    try:
        assert dll.eciPersianPrepareUsPilot(pilot) == -1
        assert dll.eciAddTextUtf8(voice, b'queued', 6, 0) == 1
        assert dll.eciPersianPrepareUsPilot(voice) == -1
        assert dll.eciClearInput(voice) == 1
        assert dll.eciPersianPrepareUsPilot(voice) == 0
        assert dll.eciPersianPrepareUsPilot(voice) == 0
        for rate in range(7):
            for gender in (0, 1):
                for speed in (65, 100):
                    for clock in (0, 1, 0, 2):
                        settings = ((5, rate), (0x1010, gender), (0x100d, 35 if gender else 50),
                                    (0x1007, speed), (0x1008, 72 if gender else 65),
                                    (0x1009, 37), (0x100a, 80), (0x100e, 2), (0x100f, 3),
                                    (0x100c, 1), (0x1015, 1), (0x1016, 1), (0x101b, clock))
                        for handle in (voice, pilot):
                            for key, value in settings:
                                assert dll.eciSetParam(handle, key, value) >= 0
                        for text, mode in (('ظهور ناگهانی', 0), ('سلام دنیا؟', 0),
                                           ('۱۲', 0), ('salaam donyaa.', 1)):
                            old = render(text, mode, True)
                            new = render(text, mode, False)
                            assert old[0] == 0 and new == old, (rate, gender, speed, clock, text, old[0], new[0])
                            checks += 1
        # A host may omit the explicit preload; first speech must still use
        # the same US source and keep the same callback and index positions.
        reference = render('سلام دنیا', 0, True)
        previous_voice = voice
        voice = dll.eciNewEx(0x90000)
        assert voice
        try:
            dll.eciRegisterCallback(voice, receive, None)
            assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
            for key, value in settings:
                assert dll.eciSetParam(voice, key, value) >= 0
            assert render('سلام دنیا', 0, False) == reference
            checks += 1
        finally:
            dll.eciDelete(voice)
            voice = previous_voice
        # Normal, character and explicit phone runs use the same owned pilot.
        for text, mode, runs in (
                ('ب', 0, ((len('ب'.encode()), 1),)),
                ('سلام ب دنیا', 0, ((len('سلام '.encode()), 0),
                                    (len('سلام ب'.encode()), 1), (len('سلام ب دنیا'.encode()), 0))),
                ('`[.0sA.1lam]', 0, ((len('`[.0sA.1lam]'), 2),))):
            old, new = render(text, mode, True, runs=runs), render(text, mode, False, runs=runs)
            assert old[0] == 0 and new == old, (text, old[0], new[0])
            checks += 1
        # Real Hz and WPM must follow voice changes without a new handle.
        for handle in (voice, pilot):
            assert dll.eciSetParam(handle, 8, 1) >= 0
            assert dll.eciSetVoiceParam(handle, 0, 2, 165) >= 0
            assert dll.eciSetParam(handle, 0x1017, 240) >= 0
        assert render('سلام دنیا', 0, True) == render('سلام دنیا', 0, False)
        # Unsupported mixed prose remains a no-output rejection, not an
        # accidental fallback to the Finnish contour.
        assert render('سلام NVDA دنیا', 0, False)[:3] == (1, b'', ())
        assert render('سلام', 0, False, maximum=1)[:3] == (-3, b'', ())
        checks += 3

        result = []
        worker = threading.Thread(target=lambda: result.append(render('سلام ' * 1000, 0, False, 120000)), daemon=True)
        worker.start(); deadline = time.monotonic() + 15
        while worker.is_alive() and not dll.eciSpeaking(voice) and time.monotonic() < deadline:
            time.sleep(.001)
        assert dll.eciSpeaking(voice), 'native Persian capture did not start'
        assert dll.eciStop(voice) == 1
        worker.join(20)
        assert not worker.is_alive() and result and result[0][0] == -4, result
        assert render('سلام دنیا', 0, True) == render('سلام دنیا', 0, False)
        checks += 1
    finally:
        if 'worker' in locals() and worker.is_alive():
            dll.eciStop(voice); worker.join(20)
        if 'worker' not in locals() or not worker.is_alive():
            dll.eciDelete(voice); dll.eciDelete(pilot)
    print(f'Passed {checks} owned US pilot, PCM, source-index, clock and cancellation comparisons.')


if __name__ == '__main__':
    main()
