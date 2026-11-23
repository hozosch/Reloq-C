"""Compare the R's isolated periodic and noise components with dev42."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay
from nvda_driver_probe import make_language_engine
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


def components(path, tag, phone, gender, clock):
    engine, scope = voice(path.resolve(), 0x90000)
    try:
        for key, value in ((SAMPLE_RATE, 3), (GENDER, gender), (NATIVE_SPEED, 65),
                           (ORIGINAL_ACOUSTIC_CLOCK, clock)):
            engine.set_parameter(key, value)
        captured = capture(engine, scope, tag)
        begin, end = (x / 10000 for x in
                      (captured.syllables[phone][0], captured.syllables[phone+1][0]))
        rendered = []
        for muted in ((2,), (7, 8)):
            frames = captured.frames()
            for frame in frames:
                for column in muted:
                    frame.controls[column] = 0
            pcm = bytearray()
            assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
            rendered.append(bytes(pcm))
        return captured, rendered, (begin, end)
    finally:
        engine.close()


def band_power(pcm, interval):
    samples = np.frombuffer(pcm, dtype='<i2').astype(float)
    start, end = interval
    segment = samples[round((start+8)*16):round(min(end-5, start+45)*16)]
    assert len(segment) > 80, (interval, len(segment))
    windowed = segment * np.hanning(len(segment))
    spectrum = np.fft.rfft(windowed, n=4096)
    frequencies = np.fft.rfftfreq(4096, 1/16000)
    return float(np.sum(np.abs(spectrum[(frequencies >= 1800) & (frequencies <= 3100)])**2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    cases = (('`[.0zo.1hur]', 3), ('`[.0fo.1ri]', 2),
             ('`[.1AzAr]', 3), ('`[.0ja.0vA.1ri]', 4),
             ('`[.0a.1zAr] `[.1he.0tin]', 3))
    for gender in (0, 1):
        for clock in (0, 1):
            for tag, phone in cases:
                old, old_parts, old_interval = components(args.previous, tag, phone, gender, clock)
                new, new_parts, new_interval = components(args.library, tag, phone, gender, clock)
                assert old.syllables == new.syllables and old_interval == new_interval
                assert old_parts[0] == new_parts[0], (tag, gender, clock, 'noise changed')
                for a, b in zip(old.frames(), new.frames()):
                    assert a.tick == b.tick
                    assert all(a.controls[c] == b.controls[c] for c in range(62) if c not in (11, 12)), (
                        tag, gender, clock, a.tick, 'source or acoustic target changed')
                old_power, new_power = band_power(old_parts[1], old_interval), band_power(new_parts[1], new_interval)
                assert old_power > 0 and new_power < old_power * .96, (
                    tag, gender, clock, old_power, new_power)
    for native_sentences in (False, True):
        owners = [make_language_engine(path.resolve(), path.resolve(), None,
                                       Path('/unavailable'), Path('/unavailable'),
                                       native_sentences=native_sentences)
                  for path in (args.previous, args.library)]
        try:
            for owner in owners:
                owner.set_parameter(SAMPLE_RATE, 3)
                owner.set_parameter(NATIVE_SPEED, 65)
            for text, expected_change in (('ظهور ناگهانی', True), ('فوری', True),
                                          ('خوری', True), ('یاوری', True),
                                          ('پنجشنبه 5 آذر heading level 4', True),
                                          ('سلام', False), ('بیرانوند', False),
                                          ('پزشکیان', False)):
                output = []
                for owner in owners:
                    pcm, marks = bytearray(), []
                    assert owner.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                                       lambda value: marks.append((value, len(pcm))))
                    output.append((bytes(pcm), marks))
                assert len(output[0][0]) == len(output[1][0]) and output[0][1] == output[1][1], text
                assert (output[0][0] != output[1][0]) == expected_change, (text, native_sentences)
        finally:
            for owner in owners:
                owner.close()
    print('Passed five voiced-R contexts, isolated noise, and both NVDA sentence routes.')


if __name__ == '__main__':
    main()
