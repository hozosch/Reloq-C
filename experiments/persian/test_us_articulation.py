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
                        help='optional dev118 library for sample and frame comparison')
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
                        expected = (140, 160) if gender else (70, 80)
                    else:
                        expected = (300, 200) if gender else (120, 100)
                    assert tuple(round(first.controls[i]) for i in (10, 14)) == expected, (
                        rate, gender, phone)
                    if previous:
                        before, old_rows = capture(previous, old_scope, tag, True)
                        assert len(actual.pcm) == len(before.pcm) and len(rows) == len(old_rows)
                        assert (actual.pcm != before.pcm) == (phone == 'A'), phone
                        for a, b in zip(rows, old_rows):
                            assert a.tick == b.tick
                            changed = (15, 16, 20) if phone == 'A' else ()
                            assert all(a.controls[i] == b.controls[i] for i in range(62)
                                       if i not in changed), (rate, gender, phone)
                        if phone == 'A':
                            assert any(any(a.controls[i] != b.controls[i] for i in changed)
                                       for a, b in zip(rows, old_rows)), (rate, gender, phone)
                    checks += 1
                for phone in 'ptkbdgmn':
                    tag = f'`[.1{phone}i]'
                    actual, rows = capture(candidate, scope, tag, True)
                    if previous:
                        before, old_rows = capture(previous, old_scope, tag, True)
                        _, us_rows = capture(us, us_scope, tag, False)
                        if phone in 'bdgmn':
                            assert actual.pcm == before.pcm, (rate, gender, phone)
                        elif phone == 'p':
                            old_af = max(row.controls[8] for row in old_rows)
                            new_af = max(row.controls[8] for row in rows)
                            us_af = max(row.controls[8] for row in us_rows)
                            assert abs(new_af-us_af) <= abs(old_af-us_af), (
                                rate, gender, phone, old_af, new_af, us_af)
                    checks += 1
                # SH and the SH component of TSH should actually carry the
                # US-like spectrum during frication; testing only dispatcher
                # variables misses the native rule's later backtracking.
                for tag in ('`[.1Si]', '`[.1Ci]', '`[.1iS]', '`[.1iC]'):
                    _, rows = capture(candidate, scope, tag, True)
                    selected = [r for r in rows if r.controls[8] >= 35 and
                                r.controls[36] < 5 and r.controls[37] >= 58 and
                                r.controls[38] >= 55]
                    assert len(selected) >= 5, (rate, gender, tag)
                    checks += 1
                for tag in ('`[.1fi]', '`[.1if]'):
                    _, rows = capture(candidate, scope, tag, True)
                    selected = [r for r in rows if r.controls[8] >= 35 and
                                r.controls[35] >= 50 and r.controls[36] >= 50]
                    assert len(selected) >= 5, (rate, gender, tag)
                    checks += 1
                if previous:
                    # L and the special Q/X carriers retain their targets;
                    # the following i is unchanged as well.
                    for tag in ('`[.1li]', '`[.1Ri]', '`[.1Gi]'):
                        _, rows = capture(candidate, scope, tag, True)
                        _, old_rows = capture(previous, old_scope, tag, True)
                        assert len(rows) == len(old_rows)
                        assert all(a.controls[i] == b.controls[i]
                                   for a, b in zip(rows, old_rows)
                                   for i in range(62)), (rate, gender, tag)
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
