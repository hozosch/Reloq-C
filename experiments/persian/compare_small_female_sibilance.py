"""Listening-only noise-bandwidth trials for small female voice models.

This replays the same German/US native frames at 32 or 48 kHz. Only active
sibilant noise bandwidths change; the production voice is not modified.
"""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys
import wave

import numpy as np
from scipy.signal import resample_poly

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, GENDER, HEAD_SIZE, NATIVE_PITCH, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, FrameReplay, NativeFrame
from compare_fa_clarity import matched_levels


def render(library, dialect, rate_code, text, width):
    engine = DirectEciEngine(library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    for param, value in ((SAMPLE_RATE, rate_code), (GENDER, 1), (HEAD_SIZE, 15),
                         (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (PROCESS_TAGS, 1)):
        engine.set_parameter(param, value)
    try:
        captured = FrameCapture(engine._dll).capture(engine, (text,), audio=False)
        if captured is None:
            raise RuntimeError('voice capture was cancelled')
        rows = captured.frames()
        before = bytes(rows)
        changed = 0
        for row in rows:
            c = row.controls
            german_small_s = c[37] >= 35 and c[39] >= 35 and c[38] < 15
            if not (row.gender == 1 and row.head < 30 and c[8] > 0 and
                    (row.s_frication or german_small_s)):
                continue
            for index in (2, 3, 4):
                if width and c[35 + index] > 0 and c[44 + index] > 0:
                    c[44 + index] = max(c[44 + index], width)
                    changed += 1
        expected = (NativeFrame * len(rows)).from_buffer_copy(before)
        for a, b in zip(expected, rows):
            for index in (46, 47, 48):
                a.controls[index] = b.controls[index]
        if bytes(expected) != bytes(rows) or width and not changed:
            raise AssertionError('unexpected frame change or missing sibilant')
        pcm = bytearray()
        if not FrameReplay(engine._dll).replay(engine, rows, (), pcm.extend, lambda _: None):
            raise RuntimeError('voice replay was cancelled')
        return bytes(pcm)
    finally:
        engine.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    narrator = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    for param, value in ((SAMPLE_RATE, 6), (GENDER, 0), (HEAD_SIZE, 50),
                         (NATIVE_SPEED, 65), (NATIVE_PITCH, 65)):
        narrator.set_parameter(param, value)
    audio, silence = bytearray(), bytes(48000)
    def say(value):
        pcm = bytearray()
        if not narrator.speak((value,), pcm.extend, lambda _: None) or not pcm:
            raise RuntimeError('cannot narrate the comparison')
        audio.extend(pcm);audio.extend(silence)
    try:
        say('Hörprobe gegen das Pfeifen. Weibliche Stimme, kleine Kopfgröße. Jeweils bisher, Versuch eins, Versuch zwei.')
        samples = (('Deutsch bei 32 Kilohertz', 0x40000, 4, 'Sechs süße Sachen. Sie sieht die Straße.'),
                   ('Deutsch bei 48 Kilohertz', 0x40000, 6, 'Sechs süße Sachen. Sie sieht die Straße.'),
                   ('Englisch bei 48 Kilohertz', 0x10000, 6, 'Six slippery stones. The ships are safe.'))
        for title, dialect, rate, phrase in samples:
            sounds = [render(args.library, dialect, rate, phrase, width) for width in (0, 1500, 2200)]
            if sounds[0] == sounds[1] or sounds[1] == sounds[2]:
                raise RuntimeError('a sibilant bandwidth candidate did not change the audio')
            sounds, _ = matched_levels(sounds)
            say(title + '.')
            for label, sound in zip(('Bisher.', 'Versuch eins.', 'Versuch zwei.'), sounds):
                say(label)
                if rate == 4:
                    data = np.frombuffer(sound, dtype='<i2').astype(float)
                    sound = np.clip(np.rint(resample_poly(data, 3, 2)), -32768, 32767).astype('<i2').tobytes()
                audio.extend(sound);audio.extend(silence)
        say('Ende des Vergleichs.')
    finally:
        narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1);out.setsampwidth(2);out.setframerate(48000);out.writeframes(audio)
    print('Wrote listening-only small-female-head comparison:', args.output)


if __name__ == '__main__':
    main()
