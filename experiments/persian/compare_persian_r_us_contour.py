"""Compare Persian R in two real NVDA planning paths, with verified US F0.

The utterances use the same language engine and sentence planner as NVDA.
Every sample must actually fit a US donor; a raw Finnish-backed capture is
never silently presented as a Persian US-contour comparison.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from compare_fa_clarity import matched_levels, render, write_wav
import continuous_driver
import continuous_plan


def actual_us_render(library, source):
    calls = []
    fit = continuous_plan.fit_us_word_pitch
    def checked(*args, **kwargs):
        result = fit(*args, **kwargs)
        calls.append(result)
        return result
    with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=checked), \
         patch.object(continuous_plan, 'fit_us_word_pitch', side_effect=checked):
        pcm, frames, _indexes, _ = render(library, 0, source)
    if not calls or not any(calls) or not frames:
        raise RuntimeError(f'No US contour was fitted to {source!r}')
    return pcm


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    narrator = DirectEciEngine(args.candidate.resolve(), dialect=0x40000,
                               encoding='utf8', native_voice=True)
    for key, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
        narrator.set_parameter(key, value)
    audio, silence = bytearray(), bytes(16000)
    def say(text):
        result = bytearray()
        if not narrator.speak((text,), result.extend, lambda _: None) or not result:
            raise RuntimeError('Cannot narrate the comparison')
        audio.extend(result)
        audio.extend(silence)
    try:
        say('Persisches R. Die Beispiele verwenden dieselbe US Kontur wie das Add-on.')
        for label, source in (('Null, R nach F', 'صفر.'),
                              ('Null im Satz', 'این عدد صفر است.'),
                              ('R am Anfang, Rahandasi', 'راه‌اندازی.'),
                              ('R zwischen Vokalen, Bruder', 'برادر.')):
            before = actual_us_render(args.previous, source)
            after = actual_us_render(args.candidate, source)
            if before == after:
                raise RuntimeError(f'Comparison does not change {source!r}')
            (before, after), _ = matched_levels((before, after))
            say(label + '.')
            for name, pcm in (('Bisher.', before), ('Neu.', after)):
                say(name)
                audio.extend(pcm)
                audio.extend(silence)
        say('Ende des Vergleichs.')
    finally:
        narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_wav(args.output, audio)
    print('Wrote verified US-contour Persian comparison:', args.output)


if __name__ == '__main__':
    main()
