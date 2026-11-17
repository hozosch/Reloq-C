"""Create a slower spoken comparison of Persian /ae/ and British sad."""
from __future__ import annotations

import argparse
from pathlib import Path
import wave

from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


CASES = (
    ('Hundert, sad.', 'One hundred, sad.', 'صد'),
    ('Linie, khatt.', 'Line, khatt.', 'خط'),
    ('Badbakht.', 'Badbakht.', 'بدبخت'),
    ('Menschen, mardom.', 'People, mardom.', 'مردم'),
    ('Hallo, salam.', 'Hello, salam.', 'سلام'),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('previous', 'candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--speed', type=int, default=60)
    parser.add_argument('--labels', choices=('de', 'en'), default='de')
    args = parser.parse_args()
    if not 0 <= args.speed <= 250:
        parser.error('--speed must be between 0 and 250')
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    old, old_scope = voice(args.previous, 0x90000)
    fa, scope = voice(args.candidate, 0x90000)
    label_voice, label_scope = voice(args.candidate, 0x40000 if args.labels == 'de' else 0x10000)
    gb, gb_scope = voice(args.candidate, 0x10001)
    for engine in (old, fa, label_voice, gb):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, args.speed), (GENDER, 0)):
            engine.set_parameter(setting, value)
    audio = bytearray()

    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(12800))  # 0.4 seconds at 16 kHz, mono 16-bit

    def say(text):
        sample, _ = capture(label_voice, label_scope, text, False)
        append(sample.pcm)

    try:
        if args.labels == 'de':
            say('Persisch. Ä im Vergleich. Zuerst das britische Wort sad als Klangreferenz. '
                'Danach pro Beispiel erst die bisherige, dann die britische Klangfarbe.')
        else:
            say('Persian vowel comparison. First the British word sad as a sound reference. '
                'For each Persian word, first the previous sound, then the British vowel sound.')
        reference, _ = capture(gb, gb_scope, 'sad', False)
        append(reference.pcm)
        for number, (german, english, text) in enumerate(CASES, 1):
            result = frontend.analyze_direct(text)
            if result is None or len(result.parts) != 1 or result.parts[0].language != 'fa':
                raise ValueError('No directly covered Persian word: ' + text)
            tag = result.parts[0].text
            previous, a = capture(old, old_scope, tag, True)
            current, b = capture(fa, scope, tag, True)
            assert previous.pcm != current.pcm and len(previous.pcm) == len(current.pcm)
            assert len(a) == len(b) and all(x.tick == y.tick and x.controls[1] == y.controls[1]
                                            for x, y in zip(a, b)), 'timing or F0 changed'
            say(f'Beispiel {number}. {german} Bisher.' if args.labels == 'de'
                else f'Example {number}. {english} Previous.')
            append(previous.pcm)
            say('Britische Klangfarbe.' if args.labels == 'de' else 'British vowel sound.')
            append(current.pcm)
            print(number, text, tag, len(previous.pcm), flush=True)
        say('Ende des Vergleichs.' if args.labels == 'de' else 'End of comparison.')
    finally:
        frontend.close()
        for engine in (old, fa, label_voice, gb):
            engine.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
