"""Measure first PCM for the direct and US-contour Persian C paths.

The direct comparison deliberately lacks the accepted US-derived contour;
it is a timing reference, not an alternative Persian voice. Results are
measurements, not pass/fail thresholds, because hardware and load vary.
"""
from __future__ import annotations

import argparse
import ctypes as c
import json
from pathlib import Path
import statistics
import sys
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=7)
    args = parser.parse_args()
    if not 1 <= args.repeats <= 100:
        parser.error('--repeats must be between 1 and 100')
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S, B = c.c_void_p, c.c_int, c.c_size_t, c.c_ubyte
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    declarations = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)), ('eciSetParam', I, (H, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)), ('eciSynthesize', I, (H,)),
        ('eciPersianSpeakNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S),
                                         c.POINTER(I), S, c.POINTER(S))),
        ('eciPersianSpeakNativeRunsUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S),
            c.POINTER(B), S, c.POINTER(S), c.POINTER(I), S, c.POINTER(S))),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                           c.POINTER(I), S, S, c.POINTER(S))),
    )
    for name, result, arguments in declarations:
        function = getattr(dll, name)
        function.restype, function.argtypes = result, arguments
    dll.eciRegisterCallback.argtypes = (H, callback, H)
    handles = [dll.eciNewEx(language) for language in (0x90000, 0x10000, 0x40000)]
    if not all(handles):
        for handle in handles:
            if handle:
                dll.eciDelete(handle)
        raise RuntimeError('Could not create the comparison voices')
    persian, english, german = handles
    buffers, callbacks, first = [], [], []
    try:
        for handle in handles:
            buffer = (c.c_int16 * 2048)()
            buffers.append(buffer)

            @callback
            def receive(_handle, message, count, _context):
                if message == 0 and count and not first:
                    first.append(time.perf_counter())
                return 1

            callbacks.append(receive)
            dll.eciRegisterCallback(handle, receive, None)
            if dll.eciSetOutputBuffer(handle, len(buffer), buffer) != 1:
                raise RuntimeError('Could not set the comparison output buffer')
            for parameter, value in ((5, 3), (0x1007, 85), (0x1008, 65), (0x100b, 0)):
                if dll.eciSetParam(handle, parameter, value) < 0:
                    raise RuntimeError(f'Could not set comparison parameter {parameter}')

        def measure(kind, text):
            data = text.encode('utf8'); error = S(); first.clear(); started = time.perf_counter()
            if kind == 'persian_direct':
                status = dll.eciPersianSpeakNativeUtf8(persian, data, len(data), 0,
                                                        None, None, 0, c.byref(error))
                success = status == 0
            elif kind == 'persian_direct_runs':
                end, run_mode = S(len(data)), B(0)
                status = dll.eciPersianSpeakNativeRunsUtf8(persian, data, len(data), 0,
                    c.byref(end), c.byref(run_mode), 1, None, None, 0, c.byref(error))
                success = status == 0
            elif kind == 'persian_us_contour':
                status = dll.eciPersianSpeakUsNativeUtf8(persian, english, data, len(data), 0,
                                                          None, None, 0, 120000, c.byref(error))
                success = status == 0
            else:
                handle = english if kind == 'english' else german
                success = bool(dll.eciAddTextUtf8(handle, data, len(data), 0) and dll.eciSynthesize(handle))
                status = 0 if success else -1
            if not success or not first:
                raise RuntimeError(f'{kind} did not start output: status={status}, offset={error.value}')
            return (first[0] - started) * 1000

        fa = 'سلام دنیا برای راه‌اندازی رایانه باید این گزینه انتخاب شود.'
        en = 'Hello world, select this option to start the computer.'
        de = 'Hallo Welt, wähle diese Option zum Starten des Computers.'
        results = []
        for title, multiplier in (('short', 1), ('long', 5)):
            texts = {'persian_direct': ' '.join([fa] * multiplier),
                     'persian_direct_runs': ' '.join([fa] * multiplier),
                     'persian_us_contour': ' '.join([fa] * multiplier),
                     'english': ' '.join([en] * multiplier),
                     'german': ' '.join([de] * multiplier)}
            readings = {kind: [] for kind in texts}
            for repeat in range(args.repeats + 2):
                for kind, text in texts.items():
                    elapsed = measure(kind, text)
                    if repeat >= 2:
                        readings[kind].append(elapsed)
            results.append({'case': title, 'first_pcm_ms': {
                kind: round(statistics.median(values), 3) for kind, values in readings.items()}})
        print(json.dumps({'sample_rate_hz': 16000, 'repeats': args.repeats,
                          'direct_persian_uses_us_contour': False, 'results': results},
                         ensure_ascii=False, indent=2))
    finally:
        for handle in handles:
            dll.eciDelete(handle)


if __name__ == '__main__':
    main()
