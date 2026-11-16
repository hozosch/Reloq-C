"""Check scoped vowel damping and US-informed Persian articulation."""
from __future__ import annotations

import argparse
import array
import ctypes
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture


def voice(path, dialect):
    result = DirectEciEngine(path.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    result.set_parameter(PROCESS_TAGS, 1)
    result.set_parameter(NATIVE_SPEED, 98)
    scope = result._dll.eciPersianAcousticScope
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    return result, scope


def capture(engine, scope, tag, selected):
    previous = scope(int(selected))
    try:
        result = FrameCapture(engine._dll).capture(engine, (tag,))
        assert result and result.pcm
        samples = array.array('h')
        samples.frombytes(result.pcm)
        assert sum(abs(s) >= 32767 for s in samples) * 200 < len(samples), tag
        return result, result.frames()
    finally:
        assert scope(previous) == int(selected)


def first_audible(rows):
    return next(row for row in rows if row.controls[2] > 0 or row.controls[8] > 0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path,
                        help='optional dev116 library for sample and frame comparison')
    args = parser.parse_args()
    candidate, scope = voice(args.candidate, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    us, us_scope = voice(args.candidate, 0x10000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for engine in (candidate, previous, us):
                    if engine:
                        engine.set_parameter(SAMPLE_RATE, rate)
                        engine.set_parameter(GENDER, gender)
                for phone in 'aAeiou':
                    tag = f'`[.1{phone}]'
                    actual, rows = capture(candidate, scope, tag, True)
                    first = first_audible(rows)
                    if phone == 'a':
                        expected = (150, 125)
                    elif phone == 'u':
                        expected = (175, 200) if gender else (88, 100)
                    else:
                        expected = (375, 250) if gender else (150, 125)
                    assert tuple(round(first.controls[i]) for i in (10, 14)) == expected, (
                        rate, gender, phone)
                    if previous:
                        before, old_rows = capture(previous, old_scope, tag, True)
                        assert len(actual.pcm) == len(before.pcm) and len(rows) == len(old_rows)
                        assert actual.pcm != before.pcm
                        for a, b in zip(rows, old_rows):
                            assert a.tick == b.tick
                            assert all(a.controls[i] == b.controls[i] for i in range(62)
                                       if i not in (10, 14)), (rate, gender, phone)
                    checks += 1
                for phone in 'ptkbdgmn':
                    tag = f'`[.1{phone}i]'
                    actual, rows = capture(candidate, scope, tag, True)
                    if previous:
                        before, old_rows = capture(previous, old_scope, tag, True)
                        _, us_rows = capture(us, us_scope, tag, False)
                        assert len(actual.pcm) == len(before.pcm) and len(rows) == len(old_rows)
                        if phone in 'mn':
                            old_f2 = first_audible(old_rows).controls[13]
                            new_f2 = first_audible(rows).controls[13]
                            us_f2 = first_audible(us_rows).controls[13]
                            assert abs(new_f2-us_f2) < abs(old_f2-us_f2), (
                                rate, gender, phone, old_f2, new_f2, us_f2)
                        else:
                            old_af = max(row.controls[8] for row in old_rows)
                            new_af = max(row.controls[8] for row in rows)
                            us_af = max(row.controls[8] for row in us_rows)
                            assert abs(new_af-us_af) <= abs(old_af-us_af), (
                                rate, gender, phone, old_af, new_af, us_af)
                    checks += 1
                if previous:
                    # L and the special Q/X carriers retain all their targets;
                    # the following vowel can change B1/B2 and its transitions.
                    for tag in ('`[.1li]', '`[.1Ri]', '`[.1Gi]'):
                        _, rows = capture(candidate, scope, tag, True)
                        _, old_rows = capture(previous, old_scope, tag, True)
                        assert len(rows) == len(old_rows)
                        assert all(a.controls[i] == b.controls[i]
                                   for a, b in zip(rows, old_rows)
                                   for i in range(62) if i not in (10, 14)), (rate, gender, tag)
                        checks += 1
                    # No effect on normal Finnish, US English or German.
                    for dialect, text in ((0x90000, '`[.1li]'),
                                          (0x10000, 'A normal English sentence.'),
                                          (0x40000, 'Ein deutscher Satz.')):
                        old_other, old_other_scope = voice(args.previous, dialect)
                        new_other, new_other_scope = voice(args.candidate, dialect)
                        try:
                            for engine in (old_other, new_other):
                                engine.set_parameter(SAMPLE_RATE, rate)
                                engine.set_parameter(GENDER, gender)
                            a, _ = capture(old_other, old_other_scope, text, False)
                            b, _ = capture(new_other, new_other_scope, text, False)
                            assert a.pcm == b.pcm, (rate, gender, dialect)
                        finally:
                            old_other.close()
                            new_other.close()
                        checks += 1
    finally:
        candidate.close()
        us.close()
        if previous:
            previous.close()
    print(f'Passed {checks} Persian vowel, stop, nasal and isolation checks.')


if __name__ == '__main__':
    main()
