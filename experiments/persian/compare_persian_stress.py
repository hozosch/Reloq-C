"""Contrast current syllable prominence with morphology-informed candidates."""
import argparse
from pathlib import Path
import re
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE

CASES = (
    ('کتاب', 'Buch. Ketab.', 'Book. Ketab.', '`[.1ke.0tab]', '`[.0ke.1tab]'),
    ('خانه', 'Haus. Khane.', 'House. Khane.', "`[.1Ga.0n'e:']", "`[.0Ga.1n'e:']"),
    ('مدرسه', 'Schule. Madrese.', 'School. Madrese.', '`[.0mAd.1re.0se]', '`[.0mAd.0re.1se]'),
    ('کتابها', 'Bücher. Ketabha.', 'Books. Ketabha.', '`[.0ke.1tab.0ha]', '`[.0ke.0tab.1ha]'),
    ('می‌کنم', 'Ich mache. Mikonam.', 'I do. Mikonam.', '`[.0mi.1ko.0nAm]', '`[.1mi.0ko.0nAm]'),
    ('نمی‌کنم', 'Ich mache nicht. Nemikonam.', 'I do not. Nemikonam.', '`[.2ne.0mi.1ko.0nAm]', '`[.1ne.0mi.0ko.0nAm]'),
)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--labels', choices=('de', 'en'), required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    lib, data = args.candidate, args.data_parent
    frontend = DirectFirstPersian(lib, None, args.espeak_library, data)
    fa, scope = voice(lib, 0x90000)
    label, label_scope = voice(lib, 0x40000 if args.labels == 'de' else 0x10000)
    for item in (fa, label):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (GENDER, 0)):
            item.set_parameter(parameter, value)
    audio = bytearray()
    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(9600))
    def say(text):
        result, _ = capture(label, label_scope, text, False)
        append(result.pcm)
    try:
        if args.labels == 'de':
            say('Vergleich der Wortbetonung auf Persisch. Erst die bisherige Version, '
                'dann eine mögliche Position nach sprachwissenschaftlichen Regeln. '
                'Dabei ändern sich auch die Lautlängen. Bitte die Aussprache beurteilen.')
        else:
            say('Persian word stress comparison. First the current version, then a possible '
                'stress position informed by linguistic rules. Sound durations also change. '
                'Please judge the pronunciation.')
        for number, (text, de, en, expected, alternative) in enumerate(CASES, 1):
            actual = frontend.analyze_direct(text).parts[0].text
            assert actual == expected
            assert re.sub(r'\.[012]', '.', actual) == re.sub(r'\.[012]', '.', alternative)
            before, _ = capture(fa, scope, actual, True)
            after, _ = capture(fa, scope, alternative, True)
            assert before.pcm != after.pcm
            say((f'Beispiel {number}. ' + de + ' Bisher.') if args.labels == 'de'
                else (f'Example {number}. ' + en + ' Current.'))
            append(before.pcm)
            say('Vergleich.' if args.labels == 'de' else 'Comparison.')
            append(after.pcm)
            print(number, text, len(before.pcm), len(after.pcm))
        say('Ende des Vergleichs.' if args.labels == 'de' else 'End of comparison.')
    finally:
        frontend.close()
        fa.close()
        label.close()
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
