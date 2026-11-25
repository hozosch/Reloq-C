"""Check German Main pronunciations and hyphen aliases in quotations."""
import argparse
from pathlib import Path
from tempfile import TemporaryDirectory

from _reloq_eci_direct import CAMEL_CASE, PROCESS_TAGS, DirectEciEngine, EciIndex, EciText
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    pairs = (('"', '"'), ('„', '“'), ('“', '”'), ('«', '»'),
             ('»', '«'), ('‚', '‘'), ("'", "'"))
    with TemporaryDirectory() as temporary:
        root = Path(temporary)
        for name, value in (('text', 'Ei-Fon'), ('phones', "`[.1'aj'.2fon]")):
            folder = root / name
            folder.mkdir()
            (folder / 'DEUmain.dic').write_bytes(('iPhone\t' + value + '\n').encode('cp1252'))
            engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                                     encoding='utf8', native_voice=True)
            plain = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                                    encoding='utf8', native_voice=True)
            try:
                engine.set_dictionary_path(folder)
                for tags in (0, 1):
                    for camel in (0, 1):
                        engine.set_parameter(PROCESS_TAGS, tags)
                        engine.set_parameter(CAMEL_CASE, camel)
                        for left, right in pairs:
                            source = left + 'iPhone' + right
                            # For a text alias the target must be planned
                            # exactly like that same written hyphenated text.
                            # For a phonetic entry compare against ordinary
                            # quotes, where native Main lookup already works.
                            target = (left + 'Ei-Fon' + right if name == 'text'
                                      else '"iPhone"')
                            for prefix, suffix in (('', ''), ('Das ', ' ist neu.')):
                                actual = render(engine, [prefix, EciIndex(11), source, EciIndex(12), suffix])
                                expected = render(engine, [prefix, EciIndex(11), target, EciIndex(12), suffix])
                                assert actual == expected, (name, tags, camel, left, prefix)
                                checks += 1
                        assert 'fon' in engine._phonemes_for_probe('„iPhone“')
                        assert 'fo.0n@' not in engine._phonemes_for_probe('„iPhone“')
                        checks += 1
                # Quoted text with no matching Main entry must retain its
                # previous behavior and explicit character reading stays intact.
                engine.set_parameter(CAMEL_CASE, 0)
                plain.set_parameter(CAMEL_CASE, 0)
                for item in ('„Ein anderes Wort“ ist hier.', '»Keine Änderung«'):
                    assert render(engine, [item]) == render(plain, [item]), (name, item)
                    checks += 1
                assert render(engine, [EciText('iPhone', character_mode=True)]) == \
                    render(plain, [EciText('iPhone', character_mode=True)])
                checks += 1
            finally:
                engine.close()
                plain.close()
    print(f'German Main quotation behavior: {checks} pronunciation, prosody and source-index checks passed')


if __name__ == '__main__':
    main()
