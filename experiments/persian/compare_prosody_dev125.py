"""Make controlled, spoken comparisons of vowel duration and word stress."""
import argparse
from pathlib import Path
import re
import wave

from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('previous', 'candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    old, old_scope = voice(args.previous, 0x90000)
    new, new_scope = voice(args.candidate, 0x90000)
    de, de_scope = voice(args.candidate, 0x40000)
    for engine in (old, new, de):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 98), (GENDER, 0)):
            engine.set_parameter(setting, value)
    audio = bytearray()

    def say(engine, scope, value, selected):
        sample, _ = capture(engine, scope, value, selected)
        audio.extend(sample.pcm)
        audio.extend(bytes(9600))

    def tags(text):
        result = frontend.analyze_direct(text)
        if result is None or len(result.parts) != 1:
            raise ValueError('No confirmed direct reading: ' + text)
        return result.parts[0].text

    try:
        say(de, de_scope, 'Vergleich für das persische Timing. '
            'Zuerst die Vokaldauer bei gleicher Wortbetonung. '
            'Pro Beispiel erst bisher, dann länger.', False)
        for description, text in (('Hallo Welt', 'سلام دنیا'), ('Buch', 'کتاب'),
                                  ('Badbakht', 'بدبخت'), ('Ich mache', 'می‌کنم')):
            value = tags(text)
            say(de, de_scope, description + '. Bisher.', False)
            say(old, old_scope, value, True)
            say(de, de_scope, 'Länger.', False)
            say(new, new_scope, value, True)
        say(de, de_scope, 'Nun die Wortbetonung. Pro Beispiel erst die bisherige '
            'automatische Position, dann ein Vergleich nach den untersuchten Regeln. '
            'Beide verwenden die gleiche neue Vokaldauer.', False)
        cases = (
            ('Buch', 'کتاب', '`[.1ke.0tab]', '`[.0ke.1tab]'),
            ('Haus', 'خانه', "`[.1Ga.0n'e:']", "`[.0Ga.1n'e:']"),
            ('Schule', 'مدرسه', '`[.0mAd.1re.0se]', '`[.0mAd.0re.1se]'),
            ('Bücher', 'کتابها', '`[.0ke.1tab.0ha]', '`[.0ke.0tab.1ha]'),
            ('Ich mache', 'می‌کنم', '`[.0mi.1ko.0nAm]', '`[.1mi.0ko.0nAm]'),
            ('Ich mache nicht', 'نمی‌کنم', '`[.2ne.0mi.1ko.0nAm]', '`[.1ne.0mi.0ko.0nAm]'),
        )
        for description, text, expected, alternate in cases:
            original = tags(text)
            if original != expected or re.sub(r'\.[012]', '.', original) != re.sub(r'\.[012]', '.', alternate):
                raise ValueError('The reference pronunciation changed: ' + text + ' ' + original)
            say(de, de_scope, description + '. Bisher.', False)
            say(new, new_scope, original, True)
            say(de, de_scope, 'Vergleich.', False)
            say(new, new_scope, alternate, True)
        say(de, de_scope, 'Ende des Vergleichs.', False)
    finally:
        frontend.close()
        old.close()
        new.close()
        de.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
