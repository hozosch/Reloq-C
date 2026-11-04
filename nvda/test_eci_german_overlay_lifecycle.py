"""Exercise the opt-in native German lexicon overlay; PCM stays in RAM."""
import argparse
import ctypes
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine
from test_eci_dictionary_semantics import capture

ROOT = Path(__file__).resolve().parents[1]


def lexicon(path, key, spr):
    path.write_bytes(b'RELOQ-DE-SPR-1\n' + key.encode('cp1252').hex().upper().encode()
                     + b'\t' + spr.encode('cp1252').hex().upper().encode() + b'\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, default=ROOT / 'eci/libreloq_eci.so')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        dictionary = root / 'dictionary'
        dictionary.mkdir()
        rootfile = dictionary / 'DEURoot.dic'
        rootfile.write_bytes(b'bestand\t`[.1hal.o]\n')
        first, second = root / 'first.rlex', root / 'second.rlex'
        lexicon(first, 'Sonderstamm', '`[.1tEst]')
        lexicon(second, 'Sonderstamm', '`[.1kAt]')
        engine = DirectEciEngine(args.library, dialect=0x40000,
                                 native_voice=True, encoding='utf8')
        try:
            load = engine._dll.eciLoadLexicon
            load.argtypes = (ctypes.c_void_p, ctypes.c_char_p)
            load.restype = ctypes.c_int
            engine.set_dictionary_path(dictionary)
            def say(word):
                return capture(engine, [word + '.'])
            original, preserved = say('Sonderstamm'), say('Bestand')
            assert load(engine._handle, str(first).encode()) == 1
            first_output = [say(word) for word in ('Sonderstamm', 'ETI-Sonderstamm')]
            assert first_output[0] != original
            assert say('Bestand') == preserved
            rootfile.unlink()
            assert load(engine._handle, str(root / 'missing.rlex').encode()) == 0
            assert [say(word) for word in ('Sonderstamm', 'ETI-Sonderstamm')] == first_output
            assert say('Bestand') == preserved, 'native dictionary snapshot was reloaded'
            assert load(engine._handle, str(second).encode()) == 1
            assert say('Sonderstamm') != first_output[0]
            assert say('Bestand') == preserved
            assert load(engine._handle, None) == 1
            assert say('Sonderstamm') == original
            assert say('Bestand') == preserved
        finally:
            engine.close()
    print('German native overlay: load, compound, failed reload, replacement, clear and retained dictionary snapshot passed')


if __name__ == '__main__':
    main()
