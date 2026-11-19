"""Check a real aspiration interval after Persian /l/, not just the tag."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from statistics import median

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_us_articulation import voice


H_WORDS = ('`[.0ka.0nal.1ha]', '`[.0pa.0til.1ha]',
           '`[.0ho.0tel.1ha]', '`[.0al.1hi]')
UNAFFECTED = ('`[.0ka.0nal.1a]', '`[.0ka.0nam.1ha]',
              '`[.0ka.0na.1ha]', '`[.1ha]', '`[.1baz.0hAm]')


def capture(engine, scope, tag, selected=True):
    previous = scope(int(selected))
    try:
        result = FrameCapture(engine._dll).capture(engine, (tag,), phone_segments=True)
        assert result and result.pcm and len(result.syllables) >= 2, tag
        return result
    finally:
        assert scope(previous) == int(selected)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checked = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                for speed in (40, 65, 100):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for tag in H_WORDS:
                        result = capture(current, scope, tag)
                        # The same language core without the Persian L
                        # adaptation supplies the original contextual h.
                        donor = capture(current, scope, tag, False)
                        assert len(result.syllables) >= 3 and len(donor.syllables) >= 3, tag
                        # The final phone is the vowel after h; the extra last
                        # segment is the trailing pause. The l's actual end
                        # and vowel's start bound the h-only transition.
                        start = result.syllables[-3][1] / 10000.0
                        end = result.syllables[-2][0] / 10000.0
                        assert 10 <= end-start <= 200, (tag, rate, gender, speed, start, end)
                        rows = [r for r in result.frames()
                                if start+(end-start)*.3 <= r.tick <= start+(end-start)*.7]
                        donor_start = donor.syllables[-3][1] / 10000.0
                        donor_end = donor.syllables[-2][0] / 10000.0
                        donor_rows = [r for r in donor.frames()
                                      if donor_start+(donor_end-donor_start)*.3 <= r.tick <=
                                         donor_start+(donor_end-donor_start)*.7]
                        assert rows and donor_rows, (tag, rate, gender, head, speed)
                        for control, name in ((2, 'voicing'), (7, 'aspiration')):
                            value=median(r.controls[control] for r in rows)
                            original=median(r.controls[control] for r in donor_rows)
                            assert abs(value-original)<=3, (
                                tag, rate, gender, head, speed, name, value, original)
                        if previous:
                            old = capture(previous, old_scope, tag)
                            assert old.syllables == result.syllables and old.pcm != result.pcm, (
                                tag, rate, gender, head, speed, 'changed timing or unchanged audio')
                            before=[(r.tick, tuple(r.controls)) for r in result.frames() if r.tick<start]
                            prior=[(r.tick, tuple(r.controls)) for r in old.frames() if r.tick<start]
                            assert before==prior, (tag, rate, gender, head, speed, 'preceding l changed')
                        checked += 1
                    if previous:
                        for tag in UNAFFECTED:
                            a = capture(current, scope, tag)
                            b = capture(previous, old_scope, tag)
                            assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (tag, rate, gender, speed)
                        # Finnish and non-Persian callers do not enable the
                        # Persian acoustic scope, including for the same tag.
                        for tag in H_WORDS[:1]:
                            a = capture(current, scope, tag, False)
                            b = capture(previous, old_scope, tag, False)
                            assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (tag, 'Finnish scope')
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checked} /l/ + /h/ acoustic intervals across voices, speeds and sample rates.')


if __name__ == '__main__':
    main()
