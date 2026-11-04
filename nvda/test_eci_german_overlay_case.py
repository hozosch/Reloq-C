"""Exact-case RLEX entries through German's native sentence and Root stages.

The caller's distinct spellings must retain their distinct pronunciations,
including in compounds, while an unmatched spelling must remain untouched.
PCM and source indices remain in memory.
"""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, DICTIONARY, PROCESS_TAGS
from test_eci_dictionary_semantics import capture


def lexicon(path, entries):
    rows = []
    for key, spr in sorted(entries, key=lambda entry: entry[0].encode('cp1252')):
        rows.append(key.encode('cp1252').hex().upper().encode() + b'\t'
                    + spr.encode('cp1252').hex().upper().encode() + b'\n')
    path.write_bytes(b'RELOQ-DE-SPR-1\n' + b''.join(rows))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    entries = (('Zup', '`[.1tEst]'), ('zup', '`[.1kAt]'),
               ('März', '`[.1hal.o]'), ('märz', '`[.1bErk]'))
    checks = 0
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        groups = (entries, entries[::2], entries[1::2], ())
        engines = []
        try:
            for i, group in enumerate(groups):
                path = root / f'case-{i}.rlex'
                lexicon(path, group)
                engines.append(DirectEciEngine(args.library.resolve(), dialect=0x40000,
                    encoding='utf8', native_voice=True, lexicon_path=path))
            both, upper, lower, empty = engines
            for spelling, selected in (('Zup', upper), ('zup', lower),
                                       ('März', upper), ('märz', lower)):
                parts = [spelling + '.', EciIndex(3)]
                assert capture(selected, parts) != capture(empty, parts), (
                    spelling, 'explicit pronunciation was not applied')
                checks += 1
            for spelling, expected in (
                ('Zup', upper), ('zup', lower), ('ZUP', empty), ('zUp', empty),
                ('März', upper), ('märz', lower), ('MÄRZ', empty),
                ('Zup-Ausgabe', upper), ('zup-Ausgabe', lower),
                ('ETI-Zup', upper), ('ETI-zup', lower)):
                for prefix, suffix in (('', '.'), ('Die ', ' wurde gelesen.')):
                    parts = [prefix, EciIndex(1), spelling, EciIndex(2), suffix, EciIndex(3)]
                    assert capture(both, parts) == capture(expected, parts), spelling
                    checks += 1
            for length in (79, 80, 100, 101, 127):
                spelling = 'a' * length
                path = root / f'long-{length}.rlex'
                lexicon(path, ((spelling, '`[.1tEst]'),))
                selected = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                    encoding='utf8', native_voice=True, lexicon_path=path)
                try:
                    parts = [spelling + '.', EciIndex(3)]
                    original = capture(selected, parts)
                    assert original != capture(empty, parts), (
                        length, 'long explicit pronunciation was not applied')
                    checks += 1
                    selected.set_parameter(DICTIONARY, 1)
                    empty.set_parameter(DICTIONARY, 1)
                    assert capture(selected, parts) == original, (
                        length, 'separate RLEX entry was disabled with ECI dictionary')
                    empty.set_parameter(DICTIONARY, 0)
                    checks += 1
                    if length == 127:
                        selected.set_parameter(DICTIONARY, 0)
                        for item in (selected,empty):item.set_parameter(PROCESS_TAGS,1)
                        assert capture(selected,['`da0 '+spelling+'.']) != capture(empty,['`da0 '+spelling+'.'])
                        assert capture(selected,['`da1 '+spelling+'.']) != capture(empty,['`da1 '+spelling+'.'])
                        empty.set_parameter(PROCESS_TAGS,0)
                        checks += 2
                finally:
                    selected.close()
            # A native Root entry wins over a separate RLEX pronunciation,
            # including words the native lexer would otherwise split.
            spelling='a'*101
            root_dir=root/'native-root';root_dir.mkdir()
            (root_dir/'DEURoot.dic').write_bytes(spelling.encode('cp1252')+b'\t`[.1hal.o]\n')
            path=root/'root-collision.rlex';lexicon(path,((spelling,'`[.1tEst]'),))
            both_long=DirectEciEngine(args.library.resolve(),dialect=0x40000,
                encoding='utf8',native_voice=True,lexicon_path=path)
            root_only=DirectEciEngine(args.library.resolve(),dialect=0x40000,
                encoding='utf8',native_voice=True)
            overlay_only=DirectEciEngine(args.library.resolve(),dialect=0x40000,
                encoding='utf8',native_voice=True,lexicon_path=path)
            try:
                both_long.set_dictionary_path(root_dir);root_only.set_dictionary_path(root_dir)
                parts=[spelling+'.',EciIndex(3)]
                assert capture(both_long,parts)==capture(root_only,parts)
                assert capture(root_only,parts)!=capture(overlay_only,parts)
                for item in (both_long,root_only,overlay_only):item.set_parameter(DICTIONARY,1)
                assert capture(both_long,parts)==capture(root_only,parts)
                empty.set_parameter(DICTIONARY,1)
                assert capture(root_only,parts)!=capture(empty,parts)
                empty.set_parameter(DICTIONARY,0)
                checks+=4
            finally:
                both_long.close();root_only.close();overlay_only.close()
        finally:
            for engine in engines:
                engine.close()
    print(f'German native overlay: {checks} case-sensitive word/compound/index checks; PCM in RAM')


if __name__ == '__main__':
    main()
