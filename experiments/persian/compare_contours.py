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
    ('Hallo Welt, Aussage.', 'Hello world, statement.', 'سلام دنیا.'),
    ('Hallo Welt, Frage.', 'Hello world, question.', 'سلام دنیا؟'),
    ('Ich spreche.', 'I am speaking.', 'صحبت می‌کنم.'),
    ('Ihr Buch ist gut.', 'Your book is good.', 'کتاب شما خوب است.'),
    ('Das ist ein Test, Aussage.', 'This is a test, statement.', 'این یک آزمایش است.'),
    ('Ist das ein Test, Frage.', 'Is this a test, question.', 'این یک آزمایش است؟'),
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
    parser.add_argument('--speed', type=int, default=60,
                        help='native speech speed (default: 60 for intelligible examples)')
    parser.add_argument('--labels', choices=('de', 'en'), default='de',
                        help='spoken labels in German or English')
    args = parser.parse_args()
    if not 0 <= args.speed <= 250:
        parser.error('--speed must be between 0 and 250')
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    fa, scope = voice(args.candidate, 0x90000)
    label_voice, label_scope = voice(args.candidate, 0x40000 if args.labels == 'de' else 0x10000)
    for engine in (fa, label_voice):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, args.speed), (GENDER, 0)):
            engine.set_parameter(setting, value)
    audio = bytearray()

    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(9600))  # 0.3 seconds at 16 kHz, mono 16-bit

    def say(text):
        sample, _ = capture(label_voice, label_scope, text, False)
        append(sample.pcm)

    try:
        if args.labels == 'de':
            say('Persisch. Vergleich der Satzmelodie. Erst bisher, dann ein Versuch '
                'mit geringeren Tonhöhenbewegungen. Laute und Wortbetonungen sind '
                'in beiden Beispielen gleich. Die Grundtonhöhe wurde angenähert.')
        else:
            say('Persian intonation comparison. First the current contour, then a '
                'version with less pitch movement. Both have the same sounds, word '
                'stress and timing. Overall pitch is approximately matched.')
        for number, (german, english, text) in enumerate(CASES, 1):
            result = frontend.analyze_direct(text)
            if result is None or len(result.parts) != 1 or result.parts[0].language != 'fa':
                raise ValueError('No directly covered Persian sentence: ' + text)
            before, after = contour_pair(fa, scope, result.parts[0].text)
            say(f'Beispiel {number}. {german} Bisher.' if args.labels == 'de'
                else f'Example {number}. {english} Current.')
            append(before)
            say('Weniger Bewegung.' if args.labels == 'de' else 'Less pitch movement.')
            append(after)
            print(number, text, result.parts[0].text, len(before), flush=True)
        say('Ende des Vergleichs.' if args.labels == 'de' else 'End of comparison.')
    finally:
        frontend.close()
        fa.close()
        label_voice.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
