"""Check acronym plurals and CamelCase through the common ECI engine."""
import argparse
from pathlib import Path
import re
from tempfile import TemporaryDirectory

from _reloq_eci_direct import CAMEL_CASE, WORD_SEGMENTATION, DirectEciEngine, EciIndex, EciText
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    for dialect in (0x10000, 0x10001, 0x40000, 0x120000):
        plurals = ('CDs', 'DVDs', 'ROMs', 'IDs', 'PDFs', 'URLs')
        if dialect in (0x10000, 0x10001):
            plurals += ('PACs', 'FAQs', 'SMSs', 'SOSes', 'NASAs', 'RAMs')
        elif dialect == 0x40000:
            plurals += ('AGBs', 'USBs', 'LKWs', 'WLANs', 'KIs')
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            def phone_groups(text):
                # Volume commands are allowed to vary without changing the
                # actual phoneme or its stress.
                return tuple(re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe(text)))
            def phones(text):
                return ''.join(re.sub(r'\.[0-3]', '', group) for group in phone_groups(text))
            if dialect == 0x40000:
                # Compare with natural independent text, not with the exact
                # internal transformation used to generate the plural.
                examples = {'CDs':'C Dehs', 'DVDs':'D V Dehs', 'PDFs':'P D Effs',
                            'URLs':'U R Els', 'AGBs':'A G Bees', 'USBs':'U S Bees',
                            'LKWs':'L K Wehs', 'WLANs':'W L A Ens', 'IDs':'Eidies'}
                for source, expected in examples.items():
                    assert phones(source) == phones(expected), (source, phones(source), phones(expected))
                    checks += 1
            elif dialect in (0x10000, 0x10001):
                # Independently stated pronunciations from issue #40. The
                # English SMS plural is not a German-language example.
                examples = {'CDs':'C Dees', 'DVDs':'D V Dees', 'ROMs':'roms',
                            'IDs':'Eye Dees', 'PDFs':'P D Effs', 'URLs':'U R Els',
                            'PACs':'packs', 'FAQs':'facks', 'SMSs':'S M Esses',
                            'super PACs':'super packs'}
                sentence = ('She was also outspent on TV advertising, with a flood of ads '
                            'from left-leaning PACs and super PACs pushing support for Colom.')
                examples[sentence] = sentence.replace('PACs', 'packs')
                for word_seg, camel in ((0, 0), (0, 1), (1, 0), (1, 1)):
                    engine.set_parameter(WORD_SEGMENTATION, word_seg)
                    engine.set_parameter(CAMEL_CASE, camel)
                    for source, expected in examples.items():
                        assert phone_groups(source) == phone_groups(expected), (hex(dialect), word_seg, camel, source)
                        assert render(engine, [source])[0] == render(engine, [expected])[0], (hex(dialect), word_seg, camel, source, 'audio differs')
                        checks += 1
                engine.set_parameter(WORD_SEGMENTATION, 0)
                engine.set_parameter(CAMEL_CASE, 0)
            baseline = {word: (phone_groups(word), render(engine, [word])[0])
                        for word in plurals}
            for word_seg, camel in ((0, 1), (1, 0), (1, 1)):
                engine.set_parameter(WORD_SEGMENTATION, word_seg)
                engine.set_parameter(CAMEL_CASE, camel)
                for word in plurals:
                    actual = (phone_groups(word), render(engine, [word])[0])
                    assert actual == baseline[word], (hex(dialect), word_seg, camel, word, baseline[word][0], actual[0])
                    checks += 1
            for source, split in (('BlindRadio', 'Blind Radio'), ('BlindRSS', 'Blind RSS'),
                                  ('myCDs', 'my CDs'), ('BlindURLs', 'Blind URLs'),
                                  ('HTTPServer', 'HTTP Server')):
                engine.set_parameter(WORD_SEGMENTATION, 0)
                engine.set_parameter(CAMEL_CASE, 0)
                expected = phone_groups(split)
                original = phone_groups(source)
                engine.set_parameter(WORD_SEGMENTATION, 1)
                assert phone_groups(source) == original, (hex(dialect), source, 'English split bypass')
                for word_seg in (0, 1):
                    engine.set_parameter(WORD_SEGMENTATION, word_seg)
                    engine.set_parameter(CAMEL_CASE, 1)
                    assert phone_groups(source) == expected, (hex(dialect), source, word_seg)
                    checks += 1
                engine.set_parameter(CAMEL_CASE, 0)
            # Index-bearing fragments inside a word stay in the original source.
            engine.set_parameter(WORD_SEGMENTATION, 0)
            plain = render(engine, ['Blind', EciIndex(57), 'Radio'])
            engine.set_parameter(CAMEL_CASE, 1)
            assert render(engine, ['Blind', EciIndex(57), 'Radio']) == plain
            checks += 1
        finally:
            engine.close()
    # An explicit pronunciation must take priority over automatic handling.
    with TemporaryDirectory() as temporary:
        root = Path(temporary)
        for dialect, prefix, word in ((0x40000, 'DEU', 'CDs'),
                                      (0x40000, 'DEU', 'SMSs'),
                                      (0x10000, 'ENU', 'SMSs'),
                                      (0x10001, 'ENG', 'SMSs')):
            folder = root / prefix
            folder.mkdir(exist_ok=True)
            (folder / (prefix + 'main.dic')).write_bytes((word + '\t`[.1tEst]\n').encode('ascii'))
            engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                engine.set_dictionary_path(folder)
                for word_seg, camel in ((0, 0), (0, 1), (1, 0), (1, 1)):
                    engine.set_parameter(WORD_SEGMENTATION, word_seg)
                    engine.set_parameter(CAMEL_CASE, camel)
                    assert 'tEst' in engine._phonemes_for_probe(word), (hex(dialect), word_seg, camel)
                    checks += 1
            finally:
                engine.close()
    # English-style SMSs must not acquire the English or German synthetic
    # plural syllable in any other native voice. The guarded input reaches
    # the underlying language unchanged and is an independent reference.
    for dialect in (0x20000, 0x20001, 0x30000, 0x30001, 0x40000,
                    0x50000, 0x60000, 0x60001, 0x70000, 0x80000,
                    0x90000, 0xa0000, 0x120000):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            for word in ('SMSs', 'SOSes', 'FAXes'):
                baseline = render(engine, [EciText(word, segmentation_guarded=True)])
                for word_seg, camel in ((0, 0), (0, 1), (1, 0), (1, 1)):
                    engine.set_parameter(WORD_SEGMENTATION, word_seg)
                    engine.set_parameter(CAMEL_CASE, camel)
                    assert render(engine, [word]) == baseline, (hex(dialect), word, word_seg, camel, 'unexpected plural')
                    checks += 1
        finally:
            engine.close()
    print(f'Native acronym and CamelCase: {checks} language, option and source-index checks passed')


if __name__ == '__main__':
    main()
