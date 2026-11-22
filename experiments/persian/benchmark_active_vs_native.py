"""Compare first audio and total work for the active and C-only FA paths.

The C-only route is experimental and does not yet support mixed English. Both
routes must produce identical PCM and index positions before their timings are
compared. This measures an in-process test player, not NVDA's event loop or a
real sound device. It is a diagnostic, not a hardware-independent threshold.
"""
from __future__ import annotations

import argparse
import ctypes as c
import json
from pathlib import Path
import statistics
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, SAMPLE_RATE
from nvda_driver_probe import PERSIAN_PROBE, US_ENGLISH, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--repeats', type=int, default=7)
    args = parser.parse_args()
    if not 1 <= args.repeats <= 50:
        parser.error('--repeats must be between 1 and 50')
    library = args.library.resolve()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(library))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    for name, result, arguments in (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciGetParam', I, (H, I)), ('eciSetParam', I, (H, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                           c.POINTER(I), S, S, c.POINTER(S))),
    ):
        fn = getattr(dll, name)
        fn.restype, fn.argtypes = result, arguments
    dll.eciRegisterCallback.argtypes = (H, callback, H)
    voice, pilot = dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)
    if not voice or not pilot:
        if voice:
            dll.eciDelete(voice)
        if pilot:
            dll.eciDelete(pilot)
        raise RuntimeError('Could not create the benchmark voices')
    output, quiet = (c.c_int16 * 2048)(), (c.c_int16 * 2048)()
    pcm, marks, first = bytearray(), [], []

    @callback
    def receive(_handle, message, count, _context):
        if message == 0 and count:
            if not first:
                first.append(time.perf_counter_ns())
            pcm.extend(c.string_at(output, count * 2))
        elif message == 2:
            marks.append((count, len(pcm)))
        return 1

    @callback
    def discard(_handle, _message, _count, _context):
        return 1

    engine = None
    try:
        for handle, buffer, cb in ((voice, output, receive), (pilot, quiet, discard)):
            dll.eciRegisterCallback(handle, cb, None)
            if dll.eciSetOutputBuffer(handle, len(buffer), buffer) != 1:
                raise RuntimeError('Could not set the benchmark output buffer')
        engine = make_language_engine(library, library, None, None, None, language=PERSIAN_PROBE.id)
        engine.set_parameter(SAMPLE_RATE, 3)
        engine.set_parameter(0x1007, 85)
        fa = engine._activate(engine._key(PERSIAN_PROBE.id, '1'))
        us = engine._activate(engine._key(US_ENGLISH.id, '1'))
        for handle, source in ((voice, fa._voice), (pilot, us)):
            for parameter in (5, 0x1007, 0x1008, 0x1009, 0x100a, 0x100b, 0x100c,
                              0x100d, 0x100e, 0x100f, 0x1010, 0x1011, 0x1015):
                value = dll.eciGetParam(source._handle, parameter)
                if value >= 0 and dll.eciSetParam(handle, parameter, value) < 0:
                    raise RuntimeError(f'Could not copy native parameter {parameter:#x}')

        cases = (('short_persian', 'سلام دنیا', True),
                 ('long_persian', 'استفاده از تنظیماتِ ذخیره‌شده‌ی فعلی هنگامِ ورود به ویندوز و دیگر صفحاتِ امن', True),
                 ('mixed', 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', False))
        report = []
        for name, text, supported in cases:
            active_first, active_total, native_first, native_total = [], [], [], []
            raw = text.encode('utf8')
            offsets, values = (S * 2)(0, len(raw)), (I * 2)(1, 2)
            for iteration in range(args.repeats + 2):
                active_pcm, active_marks, active_started = bytearray(), [], []
                start = time.perf_counter_ns()
                def feed(data):
                    if not active_started:
                        active_started.append(time.perf_counter_ns())
                    active_pcm.extend(data)
                if not engine.speak((EciIndex(1), text, EciIndex(2)), feed,
                                    lambda value: active_marks.append((value, len(active_pcm)))):
                    raise RuntimeError(f'Active speech failed: {name}')
                finished = time.perf_counter_ns()
                if not active_started:
                    raise RuntimeError(f'No active audio: {name}')
                if iteration >= 2:
                    active_first.append((active_started[0] - start) / 1e6)
                    active_total.append((finished - start) / 1e6)
                pcm.clear(); marks.clear(); first.clear()
                error = S()
                start = time.perf_counter_ns()
                status = dll.eciPersianSpeakUsNativeUtf8(
                    voice, pilot, raw, len(raw), 0, offsets, values, 2, 30000, c.byref(error))
                finished = time.perf_counter_ns()
                if not supported:
                    if status != 1 or pcm or marks:
                        raise RuntimeError(f'Mixed input was not safely rejected: {name}, {status}')
                    continue
                if status != 0 or not first or bytes(pcm) != bytes(active_pcm) or marks != active_marks:
                    raise RuntimeError(f'Native route differs from active audio or indexes: {name}, {status}')
                if iteration >= 2:
                    native_first.append((first[0] - start) / 1e6)
                    native_total.append((finished - start) / 1e6)
            median = lambda values: round(statistics.median(values), 3)
            row = {'case': name, 'active_first_pcm_ms': median(active_first),
                   'active_total_ms': median(active_total), 'native_supported': supported}
            if supported:
                row.update(native_first_pcm_ms=median(native_first), native_total_ms=median(native_total),
                           pcm_and_indexes_match=True)
            report.append(row)
        print(json.dumps({'sample_rate_hz': 16000, 'repeats': args.repeats, 'results': report},
                         ensure_ascii=False, indent=2))
    finally:
        if engine is not None:
            engine.close()
        dll.eciDelete(voice)
        dll.eciDelete(pilot)


if __name__ == '__main__':
    main()
