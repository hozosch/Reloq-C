"""Check the Spanish-inspired question contour in the private Finnish path."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

import numpy as np
from scipy.signal import butter, find_peaks, sosfiltfilt

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from test_selected_probe import KEYS, set_probe

PHRASE = '`[.0sa.1lam] `[.1do.0nja]'


def pitch_windows(pcm: bytes) -> list[tuple[float, float]]:
    """Approximate F0 in voiced 40-ms windows of the 16-kHz reference."""
    signal = np.frombuffer(pcm, dtype='<i2').astype(float)
    signal = sosfiltfilt(butter(3, [70, 550], btype='bandpass', fs=16000, output='sos'), signal)
    points = []
    for start in range(0, len(signal) - 640, 160):
        window = signal[start:start + 640]
        if np.sqrt(np.mean(window * window)) < 60:
            continue
        window = (window - np.mean(window)) * np.hanning(640)
        corr = np.correlate(window, window, mode='full')[639:]
        if not corr[0]:
            continue
        corr /= corr[0]
        peaks, _ = find_peaks(corr[38:200])
        if not len(peaks):
            continue
        peaks += 38
        best = peaks[np.argmax(corr[peaks])]
        if corr[best] >= 0.45:
            points.append(((start + 320) / 16000, 16000 / best))
    return points


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    try:
        for language in (0x90000, 0x20000, 0x10000, 0x40000):
            engine = DirectEciEngine(args.library.resolve(), dialect=language, encoding='utf8', native_voice=True)
            try:
                engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        engine.set_parameter(SAMPLE_RATE, rate)
                        engine.set_parameter(GENDER, gender)
                        for text in (PHRASE + '.', PHRASE + '?', 'salam donya.', 'salam donya?'):
                            set_probe()
                            original = render(engine, text)
                            set_probe(rise=1)
                            candidate = render(engine, text)
                            if language == 0x90000 and text.endswith('?'):
                                assert candidate != original, ('question unchanged', rate, gender, text)
                                assert len(candidate) == len(original), ('question duration changed', rate, gender, text)
                            else:
                                assert candidate == original, ('contour leaked', language, rate, gender, text)
                            checks += 1
                if language == 0x90000:
                    engine.set_parameter(SAMPLE_RATE, 3)
                    engine.set_parameter(GENDER, 0)
                    set_probe(rise=1)
                    points = pitch_windows(render(engine, PHRASE + '?'))
                    before = [p for t, p in points if .53 <= t <= .58]
                    after = [p for t, p in points if .64 <= t <= .69]
                    assert len(before) >= 3 and len(after) >= 3, ('insufficient voiced windows', points)
                    first, last = float(np.median(before)), float(np.median(after))
                    assert last > first * 1.1, ('no measurable final rise', first, last)
                    print(f'Example final vowel: approximate F0 rises from {first:.0f} to {last:.0f} Hz.')
                    checks += 1
                    for speed in (40, 98, 200):
                        engine.set_parameter(NATIVE_SPEED, speed)
                        for suffix in ('.', '?'):
                            set_probe()
                            a = render(engine, PHRASE + suffix)
                            set_probe(rise=1)
                            b = render(engine, PHRASE + suffix)
                            assert (a != b) == (suffix == '?') and len(a) == len(b), \
                                ('contour at speed', speed, suffix)
                            checks += 1
            finally:
                engine.close()
        print(f'Passed {checks} question, statement, rate and language-isolation checks.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
