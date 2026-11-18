"""Produce controlled, non-production Persian clarity listening comparisons.

All variants replay the current Persian/US-contour frames. The trial changes
only formant bandwidths, not centre frequencies, frame timing, F0, or noise
targets. This is a listening probe, not an accepted new Persian voice.
"""
from __future__ import annotations

import argparse
import ctypes
import json
import math
from pathlib import Path
import sys
from unittest.mock import patch
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import (DirectEciEngine, GENDER, NATIVE_INFLECTION,
                               NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE)
from _reloq_persian_frames import FrameReplay, NativeFrame
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE

SAMPLES = (
    ('Satz', 'سلام دنیا. این یک آزمایش است.'),
    ('Nasenlaute und U', 'من یک کتاب دارم. دو روز، سلام دنیا.'),
    ('Zahl 96', '۹۶. شماره ۹۶. ۹۶: سلام دنیا.'),
    ('Mehrsilbiges Wort', 'راه‌اندازی مجدد.'),
)
MODES = ('Bisher', 'Variante_1', 'Variante_2')
MODIFIED = {0: (), 1: (16, 18, 20), 2: (10, 14)}


def smoothstep(value, low, high):
    value = min(1.0, max(0.0, (value - low) / (high - low)))
    return value * value * (3.0 - 2.0 * value)


def adjust(rows, mode):
    """Experiment with upper or lower resonance widths during voicing.

    A smooth envelope follows voicing, and fully noisy frames stay unmodified.
    The filter's acoustic state still crosses boundaries, so neighbouring
    consonant sounds can change even though their frame targets do not.
    """
    if mode not in (0, 1, 2):
        raise ValueError('unknown listening mode')
    envelope, previous_tick, changed = 0.0, None, 0
    for row in rows:
        c = row.controls
        voiced = smoothstep(c[2], 15.0, 45.0)
        quiet_noise = 1.0 - smoothstep(c[8], 5.0, 30.0)
        target = voiced * quiet_noise
        elapsed = max(1.0, min(25.0, row.tick - previous_tick)) if previous_tick is not None else 1.0
        previous_tick = row.tick
        envelope += (target - envelope) * (1.0 - math.exp(-elapsed / 12.0))
        weight = envelope * voiced * quiet_noise
        if mode == 1:
            for index in MODIFIED[mode]:
                if c[index] > 0:
                    c[index] *= 1.0 - .40 * weight
        elif mode == 2:
            for index in MODIFIED[mode]:
                if c[index] > 0:
                    c[index] *= 1.0 + .35 * weight
        changed += bool(mode and weight > .05)
    return changed


def render(library, mode, text):
    owner = make_language_engine(library, library, None, Path('/unavailable-espeak'), Path('/unavailable'))
    for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65),
                             (NATIVE_PITCH, 65), (NATIVE_INFLECTION, 30), (GENDER, 0)):
        owner.set_parameter(parameter, value)
    engine = owner._activate(owner._key(PERSIAN_PROBE.id, '1'))
    # The samples must be directly supported; a missing reference library
    # must never silently turn a test sentence into spelled letters.
    project((text,), engine._frontend, allow_fallback=False)
    original_replay = FrameReplay.replay
    traces, changed = [], []

    def observed(self, raw_voice, rows, marks, *args, **kwargs):
        if not isinstance(rows, ctypes.Array) or getattr(rows, '_type_', None) is not NativeFrame:
            raise TypeError('unexpected native frame data')
        before = bytes(rows)
        changed.append(adjust(rows, mode))
        after = (NativeFrame * len(rows)).from_buffer_copy(before)
        allowed = MODIFIED[mode]
        for left, right in zip(after, rows):
            for index in allowed:
                left.controls[index] = right.controls[index]
        if bytes(after) != bytes(rows):
            raise AssertionError('an unrelated native frame control changed')
        traces.append((before, tuple(marks)))
        return original_replay(self, raw_voice, rows, marks, *args, **kwargs)

    data, indexes = bytearray(), []
    try:
        with patch.object(FrameReplay, 'replay', new=observed):
            if not owner.speak((text,), data.extend, indexes.append):
                raise RuntimeError('listening capture was cancelled')
    finally:
        owner.close()
    if not data or not traces or mode and not any(changed):
        raise RuntimeError('no usable native Persian replay was observed')
    return bytes(data), tuple(traces), tuple(indexes), sum(changed)


def matched_levels(samples):
    """Match each phrase's RMS with static gain and common peak headroom."""
    arrays = [np.frombuffer(item, dtype='<i2').astype(np.float64) for item in samples]
    if len({len(item) for item in arrays}) != 1:
        raise ValueError('the listening variants differ in duration')
    rms = [float(np.sqrt(np.mean(item * item))) for item in arrays]
    peaks = [float(np.max(np.abs(item))) for item in arrays]
    if any(value <= 0 for value in rms) or any(peak >= 32767 for peak in peaks):
        raise ValueError('silent or clipped input')
    target = min(rms[0], *(r * (0.95 * 32767 / p) for r, p in zip(rms, peaks)))
    gains = tuple(target / value for value in rms)
    output = [np.clip(np.rint(item * gain), -32768, 32767).astype('<i2').tobytes()
              for item, gain in zip(arrays, gains)]
    return output, gains


def write_wav(path, pcm):
    with wave.open(str(path), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(pcm)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    all_audio = [bytearray() for _ in MODES]
    grouped = []
    manifest = []
    silence = bytes(2 * 16000 // 2)
    for label, text in SAMPLES:
        captures = [render(args.library, mode, text) for mode in range(3)]
        assert all(captures[i][1:3] == captures[0][1:3] for i in (1, 2)), label
        levels, gains = matched_levels([item[0] for item in captures])
        if levels[0] == levels[1] or levels[0] == levels[2]:
            raise AssertionError('a listening candidate produced no change')
        for target, sound in zip(all_audio, levels):
            target.extend(sound)
            target.extend(silence)
        grouped.append(levels)
        manifest.append({'name': label, 'text': text,
                         'samples': len(levels[0]) // 2,
                         'linear_gain': [round(gain, 5) for gain in gains],
                         'changed_frames': [item[3] for item in captures]})
    for name, data in zip(MODES, all_audio):
        write_wav(args.output_dir / ('Persisch_Klang_' + name + '.wav'), data)

    narrator = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                               encoding='utf8', native_voice=True)
    for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65)):
        narrator.set_parameter(parameter, value)
    combined = bytearray()
    def say(text):
        pcm = bytearray()
        if not narrator.speak((text,), pcm.extend, lambda _index: None) or not pcm:
            raise RuntimeError('German listening label failed')
        combined.extend(pcm)
        combined.extend(bytes(2 * 16000 // 3))
    try:
        say('Persischer Klang. Vier Beispiele, jeweils bisher, Variante eins und Variante zwei.')
        for (label, _), sounds in zip(SAMPLES, grouped):
            say(label + '.')
            for spoken, audio in zip(('Bisher.', 'Variante eins.', 'Variante zwei.'), sounds):
                say(spoken)
                combined.extend(audio)
                combined.extend(silence)
        say('Ende des Hörvergleichs.')
    finally:
        narrator.close()
    write_wav(args.output_dir / 'Persisch_Klang_Hoervergleich.wav', combined)
    (args.output_dir / 'persian_clarity_measurements.json').write_text(
        json.dumps({'sample_rate': 16000, 'speed': 65, 'pitch': 65,
                    'inflection': 30, 'gender': 0, 'items': manifest},
                   ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(f'Wrote {len(MODES)} matched clips and a narrated comparison to {args.output_dir}')


if __name__ == '__main__':
    main()
