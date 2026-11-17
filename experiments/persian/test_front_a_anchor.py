"""Check the British sad vowel in Persian and around Ach in particular."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PAUSE_MODE, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_british_front_a import vowel
from test_open_vowels import nucleus


def voice(path, gender, rate, dialect=0x90000):
    engine = DirectEciEngine(path.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    engine.set_parameter(PROCESS_TAGS, 1)
    engine.set_parameter(PAUSE_MODE, 1)
    engine.set_parameter(GENDER, gender)
    engine.set_parameter(SAMPLE_RATE, rate)
    scope = (getattr(engine._dll, 'eciPersianAcousticScope', None) or
             engine._dll.reloq_fa_acoustic_scope)
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    return engine, scope


def capture(engine, scope, tag, enabled=True):
    previous = scope(int(enabled))
    try:
        result = FrameCapture(engine._dll).capture(engine, (tag,))
        assert result and result.pcm and result.frames()
        return result
    finally:
        assert scope(previous) == int(enabled)


def front_nucleus(rows, reference):
    # Some examples contain two /ae/ vowels and an intervening consonant.
    # Select actual target-like voiced frames rather than the midpoint of the
    # whole utterance, which could land on a consonant transition.
    matches = [row for row in rows if row.controls[2] > 25 and row.controls[8] < 1 and
               abs(row.controls[9] - reference.controls[9]) < 4 and
               abs(row.controls[13] - reference.controls[13]) < 4]
    assert len(matches) >= 2, 'no settled front-vowel frames'
    return matches[len(matches) // 2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--baseline', type=Path, help='optional previous Persian build')
    args = parser.parse_args()
    ach = ('`[.1bAG]', '`[.1sAGt]', '`[.1bAd.0bAGt]',
           '`[.1GAt]', '`[.1GA.0te]', '`[.1zir.0GAt]', '`[.1GAG]')
    contexts = ach + ('`[.1sAd]', '`[.1bAt]', '`[.1pAt]', '`[.1nAm]',
                      '`[.1mAn]', '`[.1lAt]', '`[.1rAt]', '`[.1jAt]', '`[.1At]')
    unchanged = ('`[.1sod]', '`[.1sud]', '`[.1sid]', '`[.1sed]',
                 '`[.1sad]', '`[.1jek]', '`[.1Gu]')
    checks = 0
    for gender in (0, 1):
        for rate in (3, 6):
            current, scope = voice(args.probe, gender, rate)
            british, british_scope = voice(args.probe, gender, rate, 0x10001)
            baseline, old_scope = voice(args.baseline, gender, rate) if args.baseline else (None, None)
            try:
                reference = vowel(capture(british, british_scope, 'sad', False).frames())
                for tag in contexts:
                    sample = capture(current, scope, tag)
                    rows = sample.frames()
                    mid = front_nucleus(rows, reference)
                    for column in (4, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20):
                        assert abs(mid.controls[column] - reference.controls[column]) < 5, (
                            gender, rate, tag, column, mid.controls[column], reference.controls[column])
                    if baseline:
                        old = capture(baseline, old_scope, tag)
                        previous = old.frames()
                        assert len(sample.pcm) == len(old.pcm) and len(rows) == len(previous)
                        assert sample.pcm != old.pcm
                        assert all(a.tick == b.tick and abs(a.controls[1]-b.controls[1]) < .01
                                   for a, b in zip(rows, previous))
                    checks += 1
                if baseline:
                    for tag in unchanged:
                        assert capture(current, scope, tag).pcm == capture(baseline, old_scope, tag).pcm, tag
                    # The source override is local to /ae/, including when a
                    # different vowel follows in the same spoken sequence.
                    for tag, phone in (('`[.1A.0e]', 'e'), ('`[.1A.0do]', 'o'),
                                       ('`[.1A.0no]', 'o')):
                        after = nucleus(current, scope, tag, gender, phone)
                        before = nucleus(baseline, old_scope, tag, gender, phone)
                        for column in (4, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20):
                            assert abs(after.controls[column] - before.controls[column]) < .5, (
                                tag, gender, rate, column)
                    # The scoped adjustment must never affect ordinary Finnish.
                    for tag in ach:
                        assert capture(current, scope, tag, False).pcm == capture(baseline, old_scope, tag, False).pcm
            finally:
                current.close()
                british.close()
                if baseline:
                    baseline.close()
    print(f'Passed {checks} contextual short-A/Ach checks, voice/rate and isolation checks.')


if __name__ == '__main__':
    main()
