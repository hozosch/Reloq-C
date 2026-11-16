"""Keep Persian SH and X distinct inside words and across language switches."""
from __future__ import annotations

import argparse
from collections import Counter
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
import continuous_driver
from nvda_driver_probe import make_language_engine


def bands(rows):
    found = Counter()
    for row in rows:
        c = row.controls
        if c[8] < 30:
            continue
        if c[36] >= 65 and c[37] <= 5 and c[38] >= 50:
            found['x'] += 1
        # Both the original Finnish SH and the selected US-shaped SH have
        # energy in the third and fourth parallel bands; the special X has
        # its energy in the second and fourth bands instead.
        if c[37] >= 55 and c[38] >= 55 and c[36] < 65:
            found['sh'] += 1
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('scope-probe', 'native-library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    voice = DirectEciEngine(args.scope_probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    scope = (getattr(voice._dll, 'eciPersianAcousticScope', None) or
             voice._dll.reloq_fa_acoustic_scope)
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    try:
        voice.set_parameter(PROCESS_TAGS, 1)
        voice.set_parameter(SAMPLE_RATE, 3)
        previous = scope(1)
        try:
            for tag, expected in (('`[.1Sa]', {'sh'}), ('`[.1Ga]', {'x'}),
                                  ('`[.1GoS]', {'x', 'sh'}), ('`[.1SeS]', {'sh'}),
                                  ('`[.1Ga.0Sa]', {'x', 'sh'})):
                captured = FrameCapture(voice._dll).capture(voice, (tag,))
                actual = bands(captured.frames())
                assert set(actual) == expected, (tag, actual)
        finally:
            assert scope(previous) == 1
    finally:
        voice.close()

    engine = make_language_engine(args.scope_probe, args.native_library, None,
                                  args.espeak_library, args.data_parent)
    original = continuous_driver.selected_sentence
    observed = []
    def inspect(plan, parts, *rest, **options):
        result = original(plan, parts, *rest, **options)
        offset = 0
        for part, count in zip(parts, plan.counts):
            if part.language == 'fa':
                observed.append((part.text, bands(plan.frames[offset:offset + count])))
            offset += count
        return result
    continuous_driver.selected_sentence = inspect
    try:
        for text, expected in (('NVDA شروع', {'sh'}), ('NVDA شش', {'sh'}),
                               ('NVDA نمایش', {'sh'}), ('NVDA خوش', {'x', 'sh'}),
                               ('سلام show شروع', {'sh'})):
            observed.clear()
            pcm = bytearray()
            assert engine.speak((text,), pcm.extend, lambda _: None) and pcm
            relevant = [entry for entry in observed if 'S' in entry[0]]
            assert relevant and set(relevant[-1][1]) == expected, (text, relevant)
    finally:
        continuous_driver.selected_sentence = original
        engine.close()
    print('Passed Persian SH/X distinctions in native speech and mixed contexts.')


if __name__ == '__main__':
    main()
