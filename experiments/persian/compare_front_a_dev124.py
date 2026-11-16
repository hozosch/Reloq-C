"""Make a labelled comparison of the dev123 and dev124 Persian vowels."""
import argparse
from pathlib import Path
import wave

from direct_fa_sequence import project_resilient, without_generated_stress
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
    narrator, narrator_scope = voice(args.candidate, 0x40000)
    for engine in (old, new, narrator):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 98), (GENDER, 0)):
            engine.set_parameter(setting, value)
    pcm = bytearray()

    def say(engine, scope, text, selected):
        sample, _ = capture(engine, scope, text, selected)
        pcm.extend(sample.pcm)
        pcm.extend(bytes(12800))

    try:
        say(narrator, narrator_scope, 'Vergleich der persischen Vokale. '
            'Bei jedem Beispiel zuerst der bisherige Klang, dann die neue Variante. '
            'Beide verwenden die gleiche Wortbetonung.', False)
        samples = (('Hundert', 'صد', None), ('Badbakht', 'بدبخت', None),
                   ('Zwei', 'دو', None), ('O nach N', None, '`[.0no]'),
                   ('O nach M', None, '`[.0mo]'), ('O nach N im Wort', 'نخود', None),
                   ('O nach M im Wort', 'مهم', None), ('U in Ruz', 'روز', None),
                   ('Alamat', 'علامت', None), ('Buch und Schule', 'کتاب و مدرسه', None))
        for number, (description, text, tag) in enumerate(samples, 1):
            if tag is None:
                tag = without_generated_stress(project_resilient((text,), frontend)).tagged
            if '.1' in tag or '.2' in tag:
                raise ValueError('the stress comparison is not aligned')
            say(narrator, narrator_scope, f'Beispiel {number}: {description}. Bisher.', False)
            say(old, old_scope, tag, True)
            say(narrator, narrator_scope, 'Neu.', False)
            say(new, new_scope, tag, True)
        say(narrator, narrator_scope, 'Ende des Vergleichs.', False)
    finally:
        frontend.close()
        old.close()
        new.close()
        narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(pcm)
    print('Wrote', args.output, 'seconds', round(len(pcm) / 32000, 1))


if __name__ == '__main__':
    main()
