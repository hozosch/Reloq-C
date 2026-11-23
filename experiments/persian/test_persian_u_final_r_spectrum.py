"""Measure the periodic source in the body of final R after U, without noise."""
from __future__ import annotations

import argparse
import array
import math
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


def voice_band_power(path, gender, clock):
    engine, scope = voice(path.resolve(), 0x90000)
    try:
        for key, value in ((SAMPLE_RATE, 3), (GENDER, gender), (NATIVE_SPEED, 65),
                           (ORIGINAL_ACOUSTIC_CLOCK, clock)):
            engine.set_parameter(key, value)
        recorded = capture(engine, scope, '`[.0zo.1hur]')
        start, end = (x / 10000 for x in (recorded.syllables[3][0], recorded.syllables[4][0]))
        noise_frames = recorded.frames()
        # Separately suppress the periodic source and compare the noise-only
        # waveform itself. The ordinary voice retains both sources.
        for frame in noise_frames:
            frame.controls[2] = 0
        noise_pcm = bytearray()
        assert FrameReplay(engine._dll).replay(engine, noise_frames, (), noise_pcm.extend, lambda _: None)
        frames = recorded.frames()
        # Only this diagnostic removes the two noise sources. It measures the
        # periodic component without altering what the voice normally emits.
        for frame in frames:
            frame.controls[7] = frame.controls[8] = 0
        pcm = bytearray()
        assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
        samples = array.array('h')
        samples.frombytes(pcm)
        if sys.byteorder != 'little':
            samples.byteswap()
        # The previous adjustment faded out by 35 ms. Measure the following
        # sustained voice as well, while excluding the initial U transition.
        end = min(end, start + 75)
        region = samples[round((start + 35) * 16):round(end * 16)]
        assert len(region) > 100
        windowed = [sample * (0.5 - 0.5 * math.cos(math.tau * i / (len(region) - 1)))
                    for i, sample in enumerate(region)]
        power = 0.0
        for frequency in range(2000, 5000, 100):
            angle = math.tau * frequency / 16000
            sine, cosine = math.sin(angle), math.cos(angle)
            re = im = 0.0
            x, y = 1.0, 0.0
            for sample in windowed:
                re += sample * x
                im += sample * y
                x, y = x * cosine - y * sine, x * sine + y * cosine
            power += re * re + im * im
        return power, bytes(noise_pcm)
    finally:
        engine.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    for gender in (0, 1):
        for clock in (0, 1):
            old, old_noise = voice_band_power(args.previous, gender, clock)
            new, new_noise = voice_band_power(args.candidate, gender, clock)
            assert old > 0 and new / old < 0.9, (
                gender, clock, old, new, new / old if old else None)
            assert new_noise == old_noise, (gender, clock, 'noise waveform changed')
    print('Passed four isolated voiced final-R body and noise-waveform comparisons with both acoustic clocks.')


if __name__ == '__main__':
    main()
