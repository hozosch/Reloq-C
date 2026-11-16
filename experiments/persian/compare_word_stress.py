"""Make a labelled Persian word-stress listening comparison.

This is a bounded comparison, not an automatic Persian stress rule. Each
second version moves only one existing word's primary stress to its final
syllable, and the third clears that word's explicit stress marks. The actual
Persian phone sequence remains the same.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import wave

from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE

SAMPLES = (
    ('صحبت', 'Gespräch', '`[.1soh.0bAt]', '`[.0soh.1bAt]'),
    ('کتاب', 'Buch', '`[.1ke.0tab]', '`[.0ke.1tab]'),
    ('خانه', 'Haus', "`[.1Ga.0n'e:']", "`[.0Ga.1n'e:']"),
    ('مدرسه', 'Schule', '`[.0mAd.1re.0se]', '`[.0mAd.0re.1se]'),
    ('صحبت می‌کنم', 'Gespräch im Satz', '`[.1soh.0bAt]', '`[.0soh.1bAt]'),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    front = DirectFirstPersian(args.library, None, args.espeak_library, args.data_parent)
    fa, scope = voice(args.library, 0x90000)
    de, de_scope = voice(args.library, 0x40000)
    for engine in (fa, de):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 98), (GENDER, 0)):
            engine.set_parameter(parameter, value)
    pcm = bytearray()
    pause = bytes(16000)

    def say(engine, select, text, persian):
        result, _ = capture(engine, select, text, persian)
        pcm.extend(result.pcm)
        pcm.extend(pause)

    try:
        say(de, de_scope, 'Vergleich der persischen Wortbetonung. Zuerst die bisherige '
            'Betonung, danach die letzte Silbe und zuletzt ohne gesetzte Wortbetonung. '
            'Die Laute bleiben gleich.', False)
        for number, (text, description, original, alternate) in enumerate(SAMPLES, 1):
            analysis = front.analyze_direct(text)
            if not analysis or len(analysis.parts) != 1:
                raise ValueError(f'No complete native analysis for {text!r}')
            tagged = analysis.parts[0].text
            if tagged.count(original) != 1:
                raise ValueError(f'The current pronunciation changed for {text!r}: {tagged!r}')
            variant = tagged.replace(original, alternate, 1)
            neutral = tagged.replace(original, re.sub(r'\.[012]', '.0', original), 1)
            if re.sub(r'\.\d', '.', tagged) != re.sub(r'\.\d', '.', variant):
                raise ValueError('The comparison changed the phone sequence')
            say(de, de_scope, f'Beispiel {number}: {description}. Bisher.', False)
            say(fa, scope, tagged, True)
            say(de, de_scope, 'Letzte Silbe.', False)
            say(fa, scope, variant, True)
            say(de, de_scope, 'Ohne gesetzte Wortbetonung.', False)
            say(fa, scope, neutral, True)
            print(number, text, tagged, variant, flush=True)
        say(de, de_scope, 'Ende des Vergleichs.', False)
    finally:
        front.close()
        fa.close()
        de.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(pcm)
    print('Wrote', args.output, 'seconds', len(pcm) / 32000, flush=True)


if __name__ == '__main__':
    main()
