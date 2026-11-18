"""Compare two cautious back-vowel resonance experiments, without installing them.

The real NVDA language engine and its US pitch planner produce every sample.
Neither the E-vowel targets nor frame timing, F0, or formant centres change.
"""
from __future__ import annotations

import argparse
import math
from pathlib import Path
from unittest.mock import patch

import compare_fa_clarity as comparison
import continuous_driver
import continuous_plan
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE

SAMPLES = (
    ('Ein kurzer Satz', 'سلام دنیا. این یک آزمایش است.'),
    ('Zeichennamen', 'خطِ تیره. زیرخط.'),
    ('Zahl sechsundneunzig', '۹۶. شماره ۹۶. ۹۶: سلام دنیا.'),
    ('Mehrsilbige Wörter', 'راه‌اندازی مجدد. گزینه زمستانی ما او بود.'),
)


def adjust(rows, mode):
    """Change only resonance bandwidths on voiced low-F2 segments."""
    envelope, last_tick, changed = 0.0, None, 0
    for row in rows:
        c = row.controls
        voiced = comparison.smoothstep(c[2], 15.0, 45.0)
        quiet_noise = 1.0 - comparison.smoothstep(c[8], 5.0, 30.0)
        # Persian E has a high second formant; avoid changing its target.
        back = 1.0 - comparison.smoothstep(c[13], 1400.0, 1750.0)
        target = voiced * quiet_noise * back
        elapsed = max(1.0, min(25.0, row.tick - last_tick)) if last_tick is not None else 1.0
        last_tick = row.tick
        envelope += (target - envelope) * (1.0 - math.exp(-elapsed / 12.0))
        weight = envelope * target
        if mode == 1:
            for index in comparison.MODIFIED[mode]:
                if c[index] > 0:
                    c[index] *= 1.0 - .20 * weight
        elif mode == 2:
            for index in comparison.MODIFIED[mode]:
                if c[index] > 0:
                    c[index] *= 1.0 + .175 * weight
        changed += bool(mode and weight > .05)
    return changed


def render(library, mode, text):
    fits, fit = [], continuous_plan.fit_us_word_pitch
    def checked(*args, **kwargs):
        result = fit(*args, **kwargs)
        fits.append(result)
        return result
    with patch.object(comparison, 'adjust', new=adjust), \
         patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=checked), \
         patch.object(continuous_plan, 'fit_us_word_pitch', side_effect=checked):
        result = comparison.render(library, mode, text)
    if not fits or not any(fits):
        raise RuntimeError(f'No US contour fitted to {text!r}')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    narrator = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                               encoding='utf8', native_voice=True)
    for key, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
        narrator.set_parameter(key, value)
    audio, silence = bytearray(), bytes(16000)
    def say(text):
        data = bytearray()
        if not narrator.speak((text,), data.extend, lambda _: None) or not data:
            raise RuntimeError('German comparison label failed')
        audio.extend(data)
        audio.extend(silence)
    try:
        say('Persischer Klang. Vier Beispiele, jeweils bisher, Variante eins und Variante zwei.')
        for label, text in SAMPLES:
            samples = [render(args.library, mode, text) for mode in range(3)]
            if any(samples[0][1:3] != sample[1:3] for sample in samples[1:]):
                raise AssertionError('contour, frames or indexes differ before the isolated acoustic trial')
            sounds, _ = comparison.matched_levels([sample[0] for sample in samples])
            if len(set(sounds)) != 3:
                raise AssertionError('a listening variant did not produce a change')
            say(label + '.')
            for spoken, sound in zip(('Bisher.', 'Variante eins.', 'Variante zwei.'), sounds):
                say(spoken)
                audio.extend(sound)
                audio.extend(silence)
        say('Ende des Vergleichs.')
    finally:
        narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    comparison.write_wav(args.output, audio)
    print('Wrote equal-level comparison with verified US contours:', args.output)


if __name__ == '__main__':
    main()
