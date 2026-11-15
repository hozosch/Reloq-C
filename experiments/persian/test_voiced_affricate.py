"""Check the separate, still-unselected voiced affricate hearing candidates."""
from __future__ import annotations

import argparse
from pathlib import Path
import tempfile

import numpy as np

from shared_sentence_contour import capture
from test_d_z_geminates import friction_runs
from test_selected_probe import engine, set_confirmed_persian_probe
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def voiced_friction_frames(captured):
    controls = captured.frames['controls']
    return np.count_nonzero((controls[:, 2] > 0) & (controls[:, 8] > 1))


def unvoiced_gap_before_release(captured):
    """Return the longest gap before this example's voiced /dʒ/ release."""
    times = captured.frames['tick']
    controls = captured.frames['controls']
    releases = [start for start, _, voice in friction_runs(captured)
                if 200 < start < 500 and voice > 0]
    if len(releases) != 1:
        raise AssertionError(('unexpected voiced release', releases))
    start = releases[0]
    unvoiced = ((times >= start - 110) & (times <= start + 12)
                & (controls[:, 2] <= 0))
    edges = np.flatnonzero(np.diff(np.r_[False, unvoiced, False]))
    return max((int(times[end - 1] - times[begin])
                for begin, end in zip(edges[::2], edges[1::2])), default=0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    voice = engine(args.probe, 0x90000)
    try:
        with tempfile.TemporaryDirectory(prefix='reloq-fa-j-test-') as name:
            folder, serial = Path(name), 0

            def sample(tag, mode):
                nonlocal serial
                serial += 1
                set_confirmed_persian_probe(d=2, z=2, gem=mode)
                return capture(voice, [tag], folder / f'{serial}.f0',
                               frames=folder / f'{serial}.frames')

            checks = 0
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for param, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, 50)):
                        voice.set_parameter(param, value)
                    single = '`[.0tA.1wA.0Job]'
                    reference = sample(single, 0)
                    old = sample('`[.0tA.1wAJ.0Job]', 0)
                    rejected = sample('`[.0tA.1wAd.0Job]', 0)
                    candidates = [sample(single, mode) for mode in (1, 2, 3)]
                    assert len(reference.pcm) < len(candidates[0].pcm) < len(candidates[1].pcm) < len(candidates[2].pcm)
                    assert len({candidate.pcm for candidate in candidates}) == 3
                    assert voiced_friction_frames(candidates[1]) > voiced_friction_frames(reference) + 10
                    assert voiced_friction_frames(candidates[2]) > voiced_friction_frames(reference) + 10
                    assert all(unvoiced_gap_before_release(candidate) < unvoiced_gap_before_release(old)
                               and unvoiced_gap_before_release(candidate) < unvoiced_gap_before_release(rejected)
                               for candidate in candidates)
                    checks += 1
    finally:
        voice.close()
    print(f'Passed {checks} voiced affricate hearing-candidate comparisons.')


if __name__ == '__main__':
    main()
