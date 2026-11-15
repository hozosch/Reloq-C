"""Compare selected, isolated probe acoustics with the confirmed reference."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import contextmanager
import ctypes
from pathlib import Path
import sys
import threading

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from espeak_ng_probe import EspeakPhonemeProbe
from listening import render
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


@contextmanager
def scope(voice, active):
    fn = voice._dll.reloq_fa_acoustic_scope
    fn.argtypes, fn.restype = (ctypes.c_int,), ctypes.c_int
    previous = fn(int(active))
    if previous < 0:
        raise RuntimeError('cannot select Persian acoustic scope')
    try:
        yield
    finally:
        if fn(previous) < 0:
            raise RuntimeError('cannot restore Persian acoustic scope')


def parameters(voice, rate, gender):
    for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                             (NATIVE_PITCH, 81 if gender else 65),
                             (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
        voice.set_parameter(parameter, value)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scope-probe', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    set_confirmed_persian_probe(d=1, z=1)
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
        tagged = []
        for source in ('سلام.', 'ایران.', 'نقطه.', 'کاغذ.', 'یک.',
                       'دوازده.', 'من ساعت دارم.', 'او آمد.', 'او آنجا بود؟'):
            parts = translate_text(api, source)
            assert len(parts) == 1 and parts[0].language == 'fa'
            tagged.append(parts[0].text)
    standard = engine(args.baseline, 0x90000)
    reference = engine(args.reference, 0x90000)
    scoped = engine(args.scope_probe, 0x90000)
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for voice in (standard, reference, scoped):
                    parameters(voice, rate, gender)
                for text in tagged:
                    expected_native = render(standard, text)
                    expected_fa = render(reference, text)
                    with scope(scoped, False):
                        assert render(scoped, text) == expected_native, ('Finnish', rate, gender, text)
                        with scope(scoped, True):
                            assert render(scoped, text) == expected_fa, ('Persian', rate, gender, text)
                        assert render(scoped, text) == expected_native, ('restored', rate, gender, text)
                    count += 1
        parameters(standard, 3, 0)
        parameters(reference, 3, 0)
        baseline_audio, persian_audio = render(standard, tagged[0]), render(reference, tagged[0])
        assert baseline_audio != persian_audio
    finally:
        standard.close()
        reference.close()
        scoped.close()

    # Donor languages must remain unchanged even while the scoped setting is
    # active. Only the private Finnish-based Persian path may use it.
    for dialect, text in ((0x10000, 'father and friends.'),
                          (0x40000, 'Guten Tag.'),
                          (0x130000, 'buen diya.')):
        baseline = engine(args.baseline, dialect)
        candidate = engine(args.scope_probe, dialect)
        try:
            with scope(candidate, True):
                assert render(candidate, text) == render(baseline, text), ('donor language', dialect)
        finally:
            baseline.close()
            candidate.close()

    # Simultaneous requests on different owner threads must never borrow each
    # other's settings, even though the old probe environment remains set.
    rendezvous = threading.Barrier(2)
    def concurrent(active):
        voice = engine(args.scope_probe, 0x90000)
        try:
            parameters(voice, 3, 0)
            with scope(voice, active):
                rendezvous.wait(timeout=15)
                for _ in range(4):
                    assert render(voice, tagged[0]) == (persian_audio if active else baseline_audio)
        finally:
            voice.close()
    with ThreadPoolExecutor(max_workers=2) as pool:
        futures = [pool.submit(concurrent, active) for active in (False, True)]
        for future in futures:
            future.result()
    print(f'Passed {count} scoped/reference cases and concurrent Finnish/Persian requests.')


if __name__ == '__main__':
    main()
