"""Check the softer Persian R frication and preserve its timing and voicing."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from nvda_driver_probe import make_language_engine
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional pre-source-change reference')
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        # Keep the measured periodic voicing of the established medial R.
        # Its aspiration and frication are softened without adding a vowel.
        for key, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_SPEED, 82)):
            current.set_parameter(key, value)
        medial = capture(current, scope, '`[.0fo.1ri]')
        frames = {row.tick: row.controls for row in medial.frames()}
        assert tuple(frames[t][2] for t in (97, 107, 118)) == (5, 40, 56)
        assert 21 <= frames[97][7] <= 25
        assert 40 <= frames[107][7] <= 42
        assert 30 <= frames[118][7] <= 34
        assert 55 <= frames[107][8] <= 57, ('medial R noise', frames[107][8])
        final = capture(current, scope, '`[.0zo.1hur]')
        frames = {row.tick: row.controls for row in final.frames()}
        assert tuple(frames[227][j] for j in (2, 7, 8)) == (15, 15, 0)
        assert tuple(frames[243][j] for j in (2, 7)) == (25, 20)
        assert 50 <= frames[243][8] <= 52, ('final R noise', frames[243][8])
        checks += 2
        # The original acoustic clock must also hold the newly adjusted
        # noise level on its 5-ms grid, including across phone boundaries.
        current.set_parameter(ORIGINAL_ACOUSTIC_CLOCK, 1)
        for tag in ('`[.0fo.1ri]', '`[.0zo.1hur]', '`[.1AzAr]'):
            rows = capture(current, scope, tag).frames()
            for before, row in zip(rows, rows[1:]):
                for column in (7, 8):
                    if row.controls[column] != before.controls[column]:
                        assert row.tick % 5 == 0, (tag, row.tick, column,
                                                   before.controls[column], row.controls[column])
            checks += 1
        current.set_parameter(ORIGINAL_ACOUSTIC_CLOCK, 0)
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100, 170):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for tag in ('`[.0fo.1ri]', '`[.0zo.1hur]', '`[.1ir]', '`[.1ur] `[.1di]'):
                        actual = capture(current, scope, tag)
                        if previous:
                            baseline = capture(previous, old_scope, tag)
                            assert actual.syllables == baseline.syllables, (rate, gender, speed, tag, 'timing')
                            a, b = actual.frames(), baseline.frames()
                            assert len(a) == len(b), (rate, gender, speed, tag, 'frames')
                            reduced = False
                            for x, y in zip(a, b):
                                assert x.tick == y.tick
                                for column in (0, 1, 2, 35, 36, 37, 38, 39, 40, 41, 42):
                                    assert x.controls[column] == y.controls[column], (
                                        rate, gender, speed, tag, x.tick, column)
                                aspiration = y.controls[7] - x.controls[7]
                                assert (0 <= aspiration <= 4 if tag == '`[.0fo.1ri]'
                                        else aspiration == 0), (rate, gender, speed, tag, x.tick, aspiration)
                                delta = y.controls[8] - x.controls[8]
                                assert -0.01 <= delta <= 4.01, (rate, gender, speed, tag, x.tick, delta)
                                reduced |= delta > 0.5 or aspiration > 0.5
                            if not reduced:
                                # At the fastest setting a short medial R can
                                # already have no frication at all. There is
                                # then no noise for the new rule to reduce.
                                r_index = {'`[.0fo.1ri]': 2, '`[.0zo.1hur]': 3,
                                           '`[.1ir]': 1, '`[.1ur] `[.1di]': 1}[tag]
                                start = baseline.syllables[r_index][0] / 10000
                                end = baseline.syllables[r_index + 1][0] / 10000
                                assert all(row.controls[8] == 0 for row in b if start <= row.tick < end), (
                                    rate, gender, speed, tag, 'R source was not softened')
                        checks += 1
        owner = make_language_engine(args.library.resolve(), args.library.resolve(), None,
                                     Path('/unavailable'), Path('/unavailable'))
        try:
            for text in ('ظهور ناگهانی', 'فوری', 'دوشنبه 9 آذر heading level 4'):
                pcm, marks = bytearray(), []
                assert owner.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                                   lambda number: marks.append((number, len(pcm))))
                assert pcm and marks == [(11, 0), (12, len(pcm))], (text, marks)
                checks += 1
        finally:
            owner.close()
        if args.previous:
            # The reported examples must change in actual text playback,
            # not just in a separately captured synthetic phone trace.
            owners = [make_language_engine(path, path, None, Path('/unavailable'), Path('/unavailable'))
                      for path in (args.previous.resolve(), args.library.resolve())]
            try:
                for speed in (65, 82, 100):
                    for owner in owners:
                        for key, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_SPEED, speed)):
                            owner.set_parameter(key, value)
                    for text in ('ظهور ناگهانی', 'فوری', 'دوشنبه 9 آذر heading level 4'):
                        rendered = []
                        for owner in owners:
                            pcm, marks = bytearray(), []
                            assert owner.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                                               lambda number: marks.append((number, len(pcm))))
                            rendered.append((bytes(pcm), marks))
                        assert rendered[0][0] != rendered[1][0], (text, speed, 'no audible data changed')
                        assert len(rendered[0][0]) == len(rendered[1][0]) and rendered[0][1] == rendered[1][1]
                        checks += 1
            finally:
                for owner in owners:
                    owner.close()
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checks} Persian R frication, voicing and timing cases.')


if __name__ == '__main__':
    main()
