"""Native dictionary context, case and compound checks; all PCM stays in RAM."""
from pathlib import Path
import argparse
import hashlib
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex

ROOT = Path(__file__).resolve().parents[1]


def capture(engine, parts):
    digest, size, indices = hashlib.sha256(), [0], []

    def pcm(data):
        digest.update(data)
        size[0] += len(data)

    try:
        assert engine.speak(parts, pcm, lambda index: indices.append((index, size[0])))
    except Exception as error:
        raise AssertionError(f'Native synthesis refused {parts!r}') from error
    assert size[0]
    return digest.digest(), size[0], indices


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, default=ROOT / 'eci/libreloq_eci.so')
    args = parser.parse_args()
    count = 0
    with tempfile.TemporaryDirectory() as temporary:
        directory = Path(temporary)
        aliases = {'TokenX': 'bei', 'tokenx': 'Hallo', 'PairX': 'guten Morgen',
                   'NumberX': '504', 'UmlautX': 'Größe'}
        (directory / 'DEUmain.dic').write_bytes(b''.join(
            (key + '\t' + value + '\n').encode('cp1252') for key, value in aliases.items())
            + b'Macro\t`[.1tEst]\nCycleA\tCycleB\nCycleB\tCycleA\n')
        engine = DirectEciEngine(args.library, dialect=0x40000,
                                 experimental_generic=True, encoding='utf8',
                                 lexicon_path=ROOT / 'data/german_apple_observed.rlex')
        try:
            engine.set_dictionary_path(directory)
            for key, value in aliases.items():
                for prefix, suffix in [('Ich bin ', ' dir.'), ('', '?'), ('Test-', '.'), ('', ', denke ich.')]:
                    # The existing German frontend cannot synthesize Test-504
                    # either. Test numerical aliases in ordinary sentence
                    # contexts; compare hyphens using pronounceable words.
                    if prefix == 'Test-' and value[0].isdigit():
                        continue
                    source = [prefix, EciIndex(1), key, EciIndex(2), suffix, EciIndex(3)]
                    expected = [prefix, EciIndex(1), value, EciIndex(2), suffix, EciIndex(3)]
                    assert capture(engine, source) == capture(engine, expected), (key, prefix, suffix)
                    count += 1
            lower = capture(engine, ['macro.'])
            upper = capture(engine, ['Macro.'])
            cycle = capture(engine, ['CycleA.'])
            engine.set_dictionary_path(None)
            assert lower == capture(engine, ['macro.']), 'Main literal leaked to lowercase'
            assert upper != capture(engine, ['Macro.']), 'Main literal was not applied'
            assert cycle == capture(engine, ['CycleB.']), 'Main alias ran more than once'
            count += 3
        finally:
            engine.close()

        (directory / 'DEUmain.dic').unlink()
        spr = b'`[.1tEst]'
        (directory / 'DEURoot.dic').write_bytes(b'sonderstamm\t' + spr + b'\n')
        rlex = directory / 'standard.rlex'
        rlex.write_bytes(b'RELOQ-DE-SPR-1\n' + b'Sonderstamm'.hex().upper().encode()
                         + b'\t' + spr.hex().upper().encode() + b'\n')
        native = DirectEciEngine(args.library, dialect=0x40000,
                                 experimental_generic=True, encoding='utf8')
        standard = DirectEciEngine(args.library, dialect=0x40000,
                                   experimental_generic=True, encoding='utf8', lexicon_path=rlex)
        try:
            native.set_dictionary_path(directory)
            for word in ('Sonderstamm', 'ETI-Sonderstamm', 'Sonderstamm-Ausgabe'):
                assert capture(native, [word + '.']) == capture(standard, [word + '.']), word
                count += 1
        finally:
            native.close()
            standard.close()
    print(f'{count} native dictionary context/case/compound checks passed; no audio files written.')


if __name__ == '__main__':
    main()
