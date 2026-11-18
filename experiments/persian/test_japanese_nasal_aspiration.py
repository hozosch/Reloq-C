"""Check that Japanese moraic /N/ has no breathy release."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def nasal_frames(rows):
    return [r.controls for r in rows if r.controls[29] > r.controls[27] + 10]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    ja, scope = voice(args.candidate, 0x80000)
    old, old_scope = voice(args.previous, 0x80000) if args.previous else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (ja, old):
                        if engine:
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate),
                                               (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for text in ('3', '4', 'さん', 'よん', '`[.1aN]'):
                        current, rows = capture(ja, scope, text, False)
                        nasal = nasal_frames(rows)
                        voiced = [c for c in nasal if c[2] > 10]
                        release = [c for c in nasal if c[2] < 1 and c[8] < 1]
                        assert len(voiced) >= 2, (gender, rate, speed, text, 'nasal voice missing')
                        assert len(release) >= 2, (gender, rate, speed, text, 'nasal release missing')
                        assert max(c[7] for c in release) < 1, (gender, rate, speed, text, 'nasal breath')
                        if old:
                            before, old_rows = capture(old, old_scope, text, False)
                            old_release = [c for c in nasal_frames(old_rows) if c[2] < 1 and c[8] < 1]
                            assert max(c[7] for c in old_release) >= 35
                            assert len(current.pcm) == len(before.pcm) and len(rows) == len(old_rows)
                            for a, b in zip(rows, old_rows):
                                # Only aspiration changes; timing, voicing, nasal
                                # resonance, formants and pitch must remain intact.
                                for col in (0, 2, 8, 9, 13, 15, 17, 27, 29):
                                    assert abs(a.controls[col] - b.controls[col]) < 1, (
                                        gender, rate, speed, text, col)
                        checks += 1
                    for text in ('`[.1aN.0ha]', '`[.1aN.0sa]', '1', '2', 'ら', '`[.1ha]'):
                        current, rows = capture(ja, scope, text, False)
                        if text == '`[.1aN.0ha]':
                            assert any(r.controls[7] >= 40 and r.controls[2] < 1 for r in rows), (
                                gender, rate, speed, 'following H was suppressed')
                        if old:
                            before, _ = capture(old, old_scope, text, False)
                            assert current.pcm == before.pcm, (gender, rate, speed, text)
                        checks += 1
    finally:
        ja.close()
        if old:
            old.close()
    print(f'Passed {checks} Japanese nasal, H and isolation checks.')


if __name__ == '__main__':
    main()
