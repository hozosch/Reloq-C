"""Check the focused Persian D, voiced-S, and affricate candidates."""
from __future__ import annotations

import argparse
from pathlib import Path
import tempfile

import numpy as np

from hybrid_fa_frontend import DirectFirstPersian
from shared_sentence_contour import capture
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import native_tag_accepted
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def friction_runs(captured):
    frames, ticks = captured.frames['controls'], captured.frames['tick']
    active = frames[:, 8] > 1
    result, start = [], None
    for index, enabled in enumerate(active):
        if enabled and start is None:
            start = index
        if start is not None and (not enabled or index == len(active) - 1):
            end = index if not enabled else index + 1
            if ticks[end - 1] - ticks[start] >= 20:
                result.append((int(ticks[start]), int(ticks[end - 1]),
                               float(frames[start:end, 2].max())))
            start = None
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--native-library', type=Path, required=True)
    args = parser.parse_args()
    unused = Path('/nonexistent/reloq-reference')
    voice = engine(args.probe, 0x90000)
    try:
        with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend:
            bad = frontend.analyze_direct('بدبخت').parts[0].text
            geminate = frontend.analyze_direct('تعجب').parts[0].text
            assert bad == '`[.1bAd.0bAGt]'
            assert geminate == '`[.0tA.1wAJ.0Job]'
            for tag in (bad, geminate):
                assert native_tag_accepted(tag, voice._phonemes_for_probe(tag))
            with tempfile.TemporaryDirectory(prefix='reloq-fa-d-z-') as name:
                folder, serial = Path(name), 0

                def sample(text, *, d=0, z=0):
                    nonlocal serial
                    set_confirmed_persian_probe(d=d, z=z)
                    serial += 1
                    return capture(voice, [text], folder / f'{serial}.f0',
                                   frames=folder / f'{serial}.frames')

                checks = 0
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for param, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, 50)):
                            voice.set_parameter(param, value)
                        old, longer, longest = (sample(bad, d=mode) for mode in (0, 1, 2))
                        assert len(old.pcm) < len(longer.pcm) < len(longest.pcm)
                        assert old.pcm != longer.pcm != longest.pcm
                        for text in ("`[.1s'A:'d]", '`[.1bAd] `[.1bA]', '`[.0sA.1lam]'):
                            assert sample(text, d=0).pcm == sample(text, d=2).pcm, text
                        for text in ('`[.1zA]', '`[.0A.1zA]', '`[.1Az]'):
                            base, revised = sample(text), sample(text, z=2)
                            assert len(base.pcm) == len(revised.pcm) and base.pcm != revised.pcm
                            a, b = base.frames['controls'], revised.frames['controls']
                            assert a.shape == b.shape
                            assert np.array_equal(a[:, np.arange(62) != 8], b[:, np.arange(62) != 8])
                            assert b[:, 8].max() > a[:, 8].max()
                        assert sample('`[.1sA]', z=0).pcm == sample('`[.1sA]', z=2).pcm
                        single = sample('`[.0tA.1wA.0Job]')
                        twice = sample(geminate)
                        joined = sample('`[.0tA.1wAd.0Job]')
                        assert len(single.pcm) < len(joined.pcm)
                        old_runs, new_runs = friction_runs(twice), friction_runs(joined)
                        assert len(old_runs) == len(new_runs) + 1, (rate, gender, old_runs, new_runs)
                        assert any(200 < start < 500 and voice_level == 0 for start, _, voice_level in old_runs)
                        assert not any(200 < start < 500 and voice_level == 0 for start, _, voice_level in new_runs)
                        checks += 1
            assert frontend._worker is None
    finally:
        voice.close()
    print(f'Passed {checks} D, voiced-S and geminated affricate comparisons.')


if __name__ == '__main__':
    main()
