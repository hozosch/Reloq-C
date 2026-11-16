"""Produce an accessible, controlled Persian contour listening comparison.

The second contour is a lower-inflection experiment, not a validated model of
Persian. Only F0 is changed; stress tags, segment controls and timing match.
"""
from __future__ import annotations

import argparse
from pathlib import Path
from statistics import median
import wave

from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay


CASES = (
    ('Hallo Welt, Aussage.', 'سلام دنیا.'),
    ('Hallo Welt, Frage.', 'سلام دنیا؟'),
    ('Ich spreche.', 'صحبت می‌کنم.'),
    ('Ihr Buch ist gut.', 'کتاب شما خوب است.'),
    ('Das ist ein Test, Aussage.', 'این یک آزمایش است.'),
    ('Ist das ein Test, Frage.', 'این یک آزمایش است؟'),
)


def replay(engine, scope, frames):
    result = bytearray()
    previous = scope(1)
    try:
        assert FrameReplay(engine._dll).replay(engine, frames, (), result.extend, lambda _i: None)
    finally:
        assert scope(previous) == 1
    return bytes(result)


def contour_pair(engine, scope, tag):
    engine.set_parameter(NATIVE_INFLECTION, 30)
    first, a = capture(engine, scope, tag, True)
    engine.set_parameter(NATIVE_INFLECTION, 20)
    second, b = capture(engine, scope, tag, True)
    assert len(a) == len(b) and len(first.pcm) == len(second.pcm)
    assert all(x.tick == y.tick and all(abs(x.controls[k] - y.controls[k]) < 0.001
                                        for k in range(62) if k != 1)
               for x, y in zip(a, b)), 'inflection changed segment controls or timing'
    assert replay(engine, scope, a) == first.pcm, 'unaltered frame replay differs'
    av = [row.controls[1] for row in a if row.controls[2] > 10 and row.controls[1] > 0]
    bv = [row.controls[1] for row in b if row.controls[2] > 10 and row.controls[1] > 0]
    if not av or len(av) != len(bv):
        raise ValueError('Missing comparable voiced F0')
    scale = median(av) / median(bv)
    for row in b:
        if row.controls[1] > 0:
            row.controls[1] *= scale
    changed = replay(engine, scope, b)
    assert len(changed) == len(first.pcm) and changed != first.pcm
    return first.pcm, changed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    fa, scope = voice(args.candidate, 0x90000)
    de, de_scope = voice(args.candidate, 0x40000)
    for engine in (fa, de):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 98), (GENDER, 0)):
            engine.set_parameter(setting, value)
    audio = bytearray()

    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(9600))  # 0.3 seconds at 16 kHz, mono 16-bit

    def say(text):
        sample, _ = capture(de, de_scope, text, False)
        append(sample.pcm)

    try:
        say('Persisch. Vergleich der Satzmelodie. Erst bisher, dann ein Versuch '
            'mit geringeren Tonhöhenbewegungen. Laute und Wortbetonungen sind '
            'in beiden Beispielen gleich. Die Grundtonhöhe wurde angenähert.')
        for number, (description, text) in enumerate(CASES, 1):
            result = frontend.analyze_direct(text)
            if result is None or len(result.parts) != 1 or result.parts[0].language != 'fa':
                raise ValueError('No directly covered Persian sentence: ' + text)
            before, after = contour_pair(fa, scope, result.parts[0].text)
            say(f'Beispiel {number}. {description} Bisher.')
            append(before)
            say('Weniger Bewegung.')
            append(after)
            print(number, text, result.parts[0].text, len(before), flush=True)
        say('Ende des Vergleichs.')
    finally:
        frontend.close()
        fa.close()
        de.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
