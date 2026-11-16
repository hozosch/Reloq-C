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
                        assert actual.pcm != before.pcm, phone
                        # Vowel identity and the accepted back-A damping are
                        # retained; only rounded O/U move F2 modestly.
                        old_first = first_audible(old_rows)
                        for column in (9, 10, 14):
                            assert abs(first.controls[column] - old_first.controls[column]) < 1, (
                                rate, gender, phone, column)
                        if phone not in 'ou':
                            assert abs(first.controls[13] - old_first.controls[13]) < 1, (
                                rate, gender, phone)
                    checks += 1
                for phone in 'ptkbdgmn':
                    tag = f'`[.1{phone}i]'
                    actual, rows = capture(candidate, scope, tag, True)
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
        # A geminate marker naming a nonexistent J must not affect another
        # J's timing. A valid marker must still lengthen the selected J.
        for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_SPEED, 98)):
            candidate.set_parameter(parameter, value)
        gems = candidate._dll.eciPersianNativeGeminates
        gems.argtypes, gems.restype = (ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t), ctypes.c_int
        prior_scope = scope(1)
        try:
            def with_markers(values):
                selected = (ctypes.c_uint32 * len(values))(*values) if values else None
                assert gems(selected, len(values)) == 0
                result, _ = capture(candidate, scope, '`[.0tA.1wA.0Job]', True)
                return result.pcm
            plain = with_markers(())
            assert with_markers(((1 << 16) | 2,)) == plain
            assert len(with_markers(((1 << 16) | 1,))) > len(plain)
            checks += 2
        finally:
            assert gems(None, 0) == 0
            scope(prior_scope)
    finally:
        candidate.close()
        us.close()
        if previous:
            previous.close()
    print(f'Passed {checks} Persian vowel, stop, nasal and isolation checks.')


if __name__ == '__main__':
    main()
