"""Audition i-like consonant-context targets without changing the language core.

This is a frame-domain analogy, not a copy of eSpeak samples. Each pair uses
identical frame timing, pitch and source amplitudes. /ki, hi, mi, ni, bi/
provide the reference targets, but the following vowel keeps its own targets.
The examples are isolated syllables, not a Persian intelligibility test.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, DirectEciEngine
from _reloq_persian_frames import FrameReplay
from compare_fa_clarity import matched_levels, write_wav
from test_us_articulation import capture, voice

CASES = (('K A', 'ka'), ('K O', 'ko'), ('H A', 'ha'), ('H O', 'ho'),
         ('M A', 'ma'), ('N A', 'na'), ('B A', 'ba'))


def phase(rows, phone):
    def selected(c):
        if phone == 'k':
            return c[8] > 5
        if phone == 'h':
            return c[7] > 8 and c[2] < 10
        return c[2] > 10 and c[7] < 10 and c[8] < 5
    indices = [i for i, row in enumerate(rows) if selected(row.controls)]
    if not indices:
        raise RuntimeError(f'No consonant phase found for {phone}')
    # Only the initial run belongs to the consonant in these isolated tags.
    end = next((n for n in range(1, len(indices)) if indices[n] != indices[n-1] + 1), len(indices))
    result = indices[:end]
    if len(result) < 10:
        raise RuntimeError(f'Consonant phase too short for {phone}')
    return result


def render(engine, rows):
    pcm = bytearray()
    if not FrameReplay(engine._dll).replay(engine, rows, (), pcm.extend, lambda _i: None) or not pcm:
        raise RuntimeError('Cannot replay the listening example')
    return bytes(pcm)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    engine, scope = voice(args.library, 0x90000)
    narrator = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    for e in (engine, narrator):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
            e.set_parameter(setting, value)
    combined, silence = bytearray(), bytes(16000)
    def say(text):
        out = bytearray()
        if not narrator.speak((text,), out.extend, lambda _i: None) or not out:
            raise RuntimeError('Cannot narrate the listening example')
        combined.extend(out)
        combined.extend(silence)
    try:
        say('Vergleich von Lautübergängen. Jede Silbe zuerst bisher, dann ein i ähnlicher Versuch.')
        for label, syllable in CASES:
            phone = syllable[0]
            original, rows = capture(engine, scope, '`[.1' + syllable + ']', True)
            _, reference = capture(engine, scope, '`[.1' + phone + 'i]', True)
            changed, source = phase(rows, phone), phase(reference, phone)
            before = render(engine, rows)
            columns = tuple(range(35, 41)) if phone == 'k' else (9, 13, 15, 17, 19)
            for position, index in enumerate(changed):
                src = source[round(position * (len(source) - 1) / (len(changed) - 1))]
                # A gradual exit retains the actual following vowel and
                # avoids an abrupt filter jump at the phoneme boundary.
                weight = min(1.0, (position + 1) / 5.0, (len(changed) - position) / 15.0)
                for column in columns:
                    current = rows[index].controls[column]
                    target = reference[src].controls[column]
                    value = current + weight * (target - current)
                    # Noise amplitudes are integer-valued native controls;
                    # formant frequencies support continuous interpolation.
                    rows[index].controls[column] = round(value) if phone == 'k' else value
            after = render(engine, rows)
            if len(before) != len(after) or before == after:
                raise RuntimeError(f'Invalid comparison for {syllable}')
            levels, _ = matched_levels((before, after))
            say(label + '.')
            say('Bisher.')
            combined.extend(levels[0]); combined.extend(silence)
            say('Versuch.')
            combined.extend(levels[1]); combined.extend(silence)
        say('Ende des Vergleichs.')
    finally:
        engine.close()
        narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_wav(args.output, combined)
    print(f'Wrote {len(CASES)} matched consonant comparisons to {args.output}')


if __name__ == '__main__':
    main()
