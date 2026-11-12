"""Check acronym plurals and CamelCase through the common ECI engine."""
import argparse
from pathlib import Path
import re
from tempfile import TemporaryDirectory

from _reloq_eci_direct import CAMEL_CASE, WORD_SEGMENTATION, DirectEciEngine, EciIndex
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    plurals = ('CDs', 'DVDs', 'ROMs', 'IDs', 'PDFs', 'URLs', 'PACs', 'FAQs', 'SMSs', 'SOSes',
               'AGBs', 'USBs', 'LKWs', 'WLANs', 'NASAs', 'RAMs', 'KIs')
    checks = 0
    for dialect in (0x10000, 0x10001, 0x40000, 0x120000):
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
                            'LKWs':'L K Wehs', 'WLANs':'W L A Ens', 'IDs':'Eidies',
                            'SMSs':'S M Esses'}
                for source, expected in examples.items():
                    assert phones(source) == phones(expected), (source, phones(source), phones(expected))
                    checks += 1
            elif dialect in (0x10000, 0x10001):
                assert phones('SMSs') == phones('S M Esses'), (hex(dialect), phones('SMSs'))
                checks += 1
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
                                      (0x10000, 'ENU', 'SMSs'),
                                      (0x10001, 'ENG', 'SMSs')):
            folder = root / prefix
            folder.mkdir()
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
    print(f'Native acronym and CamelCase: {checks} language, option and source-index checks passed')


if __name__ == '__main__':
    main()
