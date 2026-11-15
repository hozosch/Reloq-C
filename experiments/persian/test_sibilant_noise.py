"""Check Persian sibilant targets and keep their native formant trajectories."""
from __future__ import annotations

import argparse
import array
import ctypes
from pathlib import Path
from statistics import median
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture


def sample(path, language, text, rate, gender, scoped):
    voice = DirectEciEngine(path.resolve(), dialect=language, encoding='utf8', native_voice=True)
    voice.set_parameter(PROCESS_TAGS, 1)
    scope = voice._dll.reloq_fa_acoustic_scope
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    try:
        voice.set_parameter(SAMPLE_RATE, rate)
        voice.set_parameter(GENDER, gender)
        previous = scope(int(scoped))
        try:
            captured = FrameCapture(voice._dll).capture(voice, (text,))
        finally:
            scope(previous)
        assert captured and captured.pcm
        pcm = array.array('h')
        pcm.frombytes(captured.pcm)
        assert sum(abs(n) >= 32767 for n in pcm) * 200 < len(pcm), (text, rate, gender)
        return list(captured.frames())
    finally:
        voice.close()


def plateaus(rows, voiced):
    return [row for row in rows if row.controls[8] >= 45 and
            (row.controls[2] > 0) == voiced]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    expected_s, expected_z = (0, 0, 0, 50, 0), (30, 50, 58, 63, 0)
    checks = 0
    for rate in (1, 3, 6):
        for gender in (0, 1):
            american = sample(args.candidate, 0x10000, 'see', rate, gender, False)
            us_s = plateaus(american, False)
            assert any(tuple(round(x) for x in row.controls[36:41]) == expected_s for row in us_s)
            for tag in ('`[.1si]', '`[.1sa]', '`[.1so]'):
                rows = sample(args.candidate, 0x90000, tag, rate, gender, True)
                active = plateaus(rows, False)
                assert any(tuple(round(x) for x in row.controls[36:41]) == expected_s for row in active), tag
                assert abs(max(row.controls[8] for row in active) -
                           max(row.controls[8] for row in us_s)) <= 6, tag
                # The active US noise band is F5. Its center matters as much
                # as the band level, and previously differed by 200 Hz.
                us_center = median(row.controls[19] for row in us_s if row.controls[8] >= 55)
                fa_center = median(row.controls[19] for row in active if row.controls[8] >= 55)
                assert abs(us_center - fa_center) <= 25, (tag, rate, gender, us_center, fa_center)
                if args.previous:
                    previous = sample(args.previous, 0x90000, tag, rate, gender, True)
                    assert len(rows) == len(previous)
                    for a, b in zip(rows, previous):
                        assert a.tick == b.tick
                        assert all(a.controls[i] == b.controls[i] for i in range(62)
                                   if i not in (8, 19) and not 36 <= i <= 40), (tag, rate, gender)
                checks += 1
            for tag in ('`[.1zi]', '`[.1za]', '`[.1ze]', '`[.1zo]', '`[.1zu]', '`[.1Az]'):
                rows = sample(args.candidate, 0x90000, tag, rate, gender, True)
                active = plateaus(rows, True)
                assert any(round(row.controls[8]) == 49 and
                           tuple(round(x) for x in row.controls[36:41]) == expected_z for row in active), tag
                if args.previous:
                    previous = sample(args.previous, 0x90000, tag, rate, gender, True)
                    assert len(rows) == len(previous)
                    for a, b in zip(rows, previous):
                        assert a.tick == b.tick
                        assert all(a.controls[i] == b.controls[i] for i in range(62)
                                   if i != 8 and not 36 <= i <= 40), (tag, rate, gender)
                checks += 1
    print(f'Passed {checks} S/Z spectra, voicing, level and unchanged-transition checks.')


if __name__ == '__main__':
    main()
