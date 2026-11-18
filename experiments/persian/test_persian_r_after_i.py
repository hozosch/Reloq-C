"""Check the narrowly scoped Persian R after I in the reported words."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='the engine before the after-I trial')
    args = parser.parse_args()
    current, scope = voice(args.candidate, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    changed_words = ("`[.0ti.1r'e:']", '`[.1zir.0GAt]')
    unchanged = ("`[.1u.0r'e:']", '`[.1i] `[.1ra]', '`[.1ir]', '`[.1ra]')
    checks = 0
    try:
        for engine in (current, previous):
            if engine:
                for key, value in ((GENDER, 0), (SAMPLE_RATE, 3), (NATIVE_SPEED, 65)):
                    engine.set_parameter(key, value)
        # These measured points are in the R transitions, not the vowel
        # centers. They catch a disabled after-I rule in a standalone DLL.
        for tag, index, low_f3, expected_f4 in ((changed_words[0], 100, 2240, 2800),
                                                 (changed_words[1], 175, 2220, 2915)):
            _, frames = capture(current, scope, tag, True)
            assert frames[index].controls[15] >= low_f3, (tag, 'after-I R F3')
            assert abs(frames[index].controls[17] - expected_f4) <= 5, (tag, 'R F4 changed')
            checks += 1
        if previous:
            for gender in (0, 1):
                for rate in (1, 3, 6):
                    for speed in (40, 98, 170):
                        for engine in (current, previous):
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                        for tag in (*changed_words, *unchanged):
                            now_pcm, now = capture(current, scope, tag, True)
                            old_pcm, old = capture(previous, old_scope, tag, True)
                            assert len(now) == len(old), (gender, rate, speed, tag, 'timing')
                            changes = [i for i, (a, b) in enumerate(zip(now, old))
                                       if abs(a.controls[15] - b.controls[15]) > .5]
                            if tag in changed_words:
                                assert changes and now_pcm.pcm != old_pcm.pcm, (gender, rate, speed, tag)
                                assert changes[0] > 10, (gender, rate, speed, tag, 'initial vowel was changed')
                                assert max(now[i].controls[15]-old[i].controls[15] for i in changes) <= (
                                    180 if gender else 151)
                            else:
                                assert not changes and now_pcm.pcm == old_pcm.pcm, (gender, rate, speed, tag)
                            # Only the R's third formant and its transitions
                            # change; pitch, source, F1/F2/F4 and timing do not.
                            for a, b in zip(now, old):
                                assert a.tick == b.tick
                                for col in range(len(a.controls)):
                                    if col != 15:
                                        assert abs(a.controls[col]-b.controls[col]) < 1, (
                                            gender, rate, speed, tag, col)
                            checks += 1
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checks} contextual after-I R and isolation checks.')


if __name__ == '__main__':
    main()
