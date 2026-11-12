"""Native Aragonese rules, language isolation, dictionaries and indices."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, DICTIONARY
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    an = DirectEciEngine(args.library.resolve(), dialect=0x130000, encoding='utf8', native_voice=True)
    es = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    checks = 0
    try:
        # Both spellings are official; test word initial, medial and final.
        for ny, enye in (('sinyal', 'siñal'), ('cabanyera', 'cabañera'),
                         ('nyapir', 'ñapir'), ('bony', 'boñ'),
                         ('Espanya', 'España'), ('NYAPIR', 'ÑAPIR'),
                         ('Nyapir', 'Ñapir'), ('siNYal', 'siÑal')):
            actual, reference = phonemes(an, ny), phonemes(an, enye)
            assert actual == reference, (ny, enye, actual, reference)
            checks += 1
        assert phonemes(es, 'sinyal') != phonemes(es, 'siñal'), 'Castilian rules were changed'
        checks += 1
        # The /sh/ acoustic test is reachable through ordinary Aragonese
        # spelling; Spanish technical x words must keep their /ks/ reading.
        for word in ('xada', 'Xada', 'la xada', 'a xada', 'xuela', 'caixa', 'aixina', 'peix'):
            spoken = phonemes(an, word)
            assert 'S' in spoken, (word, spoken)
            checks += 1
        for word in ('taxi', 'sexo', 'examen', 'xenofobo'):
            assert phonemes(an, word) == phonemes(es, word), word
            checks += 1
        # Source fragments remain in the same word; indices are delivered in
        # order and the complete PCM matches a single input fragment.
        whole, _ = render(an, ['sinyal y cabanyera.'])
        split, indices = render(an, ['sin', EciIndex(1), 'yal y caba', EciIndex(2), 'nyera.'])
        assert split == whole and [i for i, _ in indices] == [1, 2], indices
        checks += 1
        characters, _ = render(an, [EciText('ny', character_mode=True)])
        assert characters and characters != render(an, ['ny'])[0]
        checks += 1
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / 'ARAmain.dic').write_bytes(b'probexword\tcasa\nsinyal\tcasa\n')
            (folder / 'ESPmain.dic').write_bytes(b'probexword\tnaranja\n')
            before = render(an, ['probexword.', EciIndex(7)])
            an.set_dictionary_path(folder)
            es.set_dictionary_path(folder)
            for flag in (0, 1):
                an.set_parameter(DICTIONARY, flag)
                assert render(an, ['probexword.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(an, ['sinyal.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(es, ['probexword.', EciIndex(7)]) == render(es, ['naranja.', EciIndex(7)])
                checks += 3
            an.set_dictionary_path(None)
            an.set_parameter(DICTIONARY, 0)
            assert render(an, ['probexword.', EciIndex(7)]) == before
            checks += 1
    finally:
        es.close(); an.close()
    print(f'Aragonese: {checks} native pronunciation, language, character, index and dictionary checks passed')


if __name__ == '__main__':
    main()
