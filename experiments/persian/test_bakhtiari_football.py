"""Exercise the reported R resonance and Persian /u:tb/ timing contexts."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture, middle
from test_us_articulation import voice


NAME = "`[.0bAG.0ti.0j'a:'.0ri.0za.1d'e:']"
FOOTBALL = "`[.0f'u:'t.1bal]"
SHORT_U = '`[.0fut.1bal]'
UNVOICED_CLUSTER = "`[.0f'u:'t.1pal]"
CONTROLS = ("`[.0ti.1r'e:']", '`[.1zir.0GAt]', '`[.1ir]', '`[.1ri]',
            "`[.1f'u:'t] `[.1bal]", "`[.1f'u:'t] `[.1pal]",
            '`[.0i] `[.1ri]')


def duration(item, index):
    start, end = item.syllables[index]
    return (end - start) / 10000.0


def max_noise(item, index):
    start, end = (value / 10000.0 for value in item.syllables[index])
    rows = [row.controls[8] for row in item.frames() if start <= row.tick <= end]
    assert rows, (start, end)
    return max(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional immediate pre-change build')
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                # 82 corresponds to the reported NVDA rate 38. Both clocks
                # are covered along with slower and faster speech.
                for speed, clock in ((40, 0), (82, 0), (82, 1), (100, 0)):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed),
                                               (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                engine.set_parameter(key, value)
                    name = capture(current, scope, NAME)
                    plain_r = capture(current, scope, '`[.1ri]')
                    voiced_r = middle(name, 7, (15,))[0]
                    initial_r = middle(plain_r, 0, (15,))[0]
                    assert 140 <= initial_r - voiced_r <= 280, (rate, gender, head, speed, voiced_r, initial_r)

                    football = capture(current, scope, FOOTBALL)
                    short_u = capture(current, scope, SHORT_U)
                    unvoiced = capture(current, scope, UNVOICED_CLUSTER)
                    assert duration(short_u, 1) < min(duration(football, 1), duration(unvoiced, 1)), (
                        rate, gender, head, speed, 'u durations',
                        duration(short_u, 1), duration(football, 1), duration(unvoiced, 1))
                    assert 54 <= max_noise(football, 2) <= 60, (rate, gender, head, speed, 't burst')
                    if old:
                        prior_name = capture(old, old_scope, NAME)
                        assert name.syllables == prior_name.syllables
                        old_r = middle(prior_name, 7, (15, 17))
                        new_r = middle(name, 7, (15, 17))
                        assert old_r[0] - new_r[0] >= 140 and abs(old_r[1] - new_r[1]) <= 1, (
                            rate, gender, head, speed, old_r, new_r)
                        prior_football = capture(old, old_scope, FOOTBALL)
                        assert duration(football, 1) < duration(prior_football, 1)
                        assert max_noise(football, 2) > max_noise(prior_football, 2)
                        for tag in CONTROLS:
                            a, b = capture(current, scope, tag), capture(old, old_scope, tag)
                            assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (
                                rate, gender, head, speed, clock, tag, 'unrelated sound changed')
                    checks += 1
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {checks} Persian name, football, voice, rate and context cases.')


if __name__ == '__main__':
    main()
