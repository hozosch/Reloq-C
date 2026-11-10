"""Verify protected CamelCase words reach actual ECI dictionaries intact."""
import argparse
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, EciText
from _reloq_wordseg import dictionary_keys, protected_native_spans
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    with TemporaryDirectory() as temporary:
        root = Path(temporary)
        for language, prefix, word in ((0x40000, 'DEU', 'MeinWort'),
                                       (0x10000, 'ENU', 'FdOpen'),
                                       (0x10000, 'ENU', 'hello-MyWorld')):
            (root / f'{prefix}main.dic').write_bytes((word + '\t`[.1tEst]\n').encode('cp1252'))
            engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                     encoding='utf8', native_voice=True)
            try:
                sentence = f'Vor {word} danach'
                before = render(engine, [EciText(sentence)])[0]
                engine.set_dictionary_path(root)
                expected = render(engine, [EciText(sentence)])[0]
                spans = list(protected_native_spans(sentence, dictionary_keys(root)[language]))
                assert (word, True) in spans, spans
                actual = render(engine, [EciText(text) for text, _ in spans])[0]
                assert before != expected, 'the native dictionary fixture must affect pronunciation'
                assert actual == expected, (language, spans)
            finally:
                engine.close()
        # Main entries are case-sensitive; English roots also match mixed
        # case, but the engine spells all-uppercase tokens as abbreviations.
        for volume, cases in (('main', (('cgroup', True), ('CGROUP', False), ('Cgroup', False),
                                       ('cgroups', False), ('CGROUPS', False))),
                              ('Root', (('cgroup', True), ('CGROUP', False), ('Cgroup', True),
                                       ('cgroups', True), ('CGROUPS', True), ('cGroups', True),
                                       ('CGROUPING', True)))):
            folder = root / volume
            folder.mkdir()
            (folder / f'ENU{volume}.dic').write_bytes(b'cgroup\t`[.1tEst]\n')
            plain = DirectEciEngine(args.library.resolve(), dialect=0x10000, encoding='utf8', native_voice=True)
            custom = DirectEciEngine(args.library.resolve(), dialect=0x10000, encoding='utf8', native_voice=True)
            try:
                custom.set_dictionary_path(folder)
                keys = dictionary_keys(folder)[0x10000]
                for word, matches in cases:
                    changed = render(plain, [word])[0] != render(custom, [word])[0]
                    assert changed == matches and (word in keys) == matches, (volume, word)
            finally:
                plain.close()
                custom.close()
    print('Segmentation: German and English native dictionary precedence preserved in actual ECI audio')


if __name__ == '__main__':
    main()
