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
        for word in ('xada', 'Xada', 'la xada', 'a xada', 'xuela', 'caixa', 'aixina', 'peix',
                     'ixambre', 'ixe', 'ixuela', 'enxargar', 'engauxar', 'mixina', 'pixar', 'xino'):
            spoken = phonemes(an, word)
            assert 'S' in spoken, (word, spoken)
            checks += 1
        for word in ('taxi', 'sexo', 'examen', 'xenofobo', 'fixar', 'mixto', 'afixo', 'auxilio'):
            assert phonemes(an, word) == phonemes(es, word), word
            checks += 1
        assert 'ks' in phonemes(an, 'xilofaga'), phonemes(an, 'xilofaga')
        checks += 1
        for qu, cu in (('quaranta','cuaranta'), ('quadro','cuadro')):
            assert phonemes(an, qu)==phonemes(an, cu), (qu, cu)
            checks += 1
        assert phonemes(an, 'queso')==phonemes(es, 'queso')
        checks += 1
        # Number words are chosen inside the existing numeric and character
        # rules, including hundreds and the combined twenties.
        for number, word in (('8', 'ueito'), ('9', 'nueu'), ('12', 'dotze'),
                             ('13', 'tretze'), ('18', 'deciueito'), ('19', 'decinueu'),
                             ('20', 'vinte'), ('28', 'vintiueito'), ('29', 'vintinueu'),
                             ('30', 'trenta'), ('40', 'cuaranta'), ('50', 'cincuanta'),
                             ('60', 'sisanta'), ('70', 'setanta'), ('80', 'uitanta'),
                             ('90', 'novanta'), ('500', 'cincocientos'), ('900', 'nueucientos')):
            actual, reference = phonemes(an, number), phonemes(an, word)
            assert actual == reference, (number, word, actual, reference)
            checks += 1
        # Only hyphen-attached pronouns lose their own word stress. Ordinary
        # independent words and other kinds of hyphenated words stay intact.
        for sentence, expected in (('minchar-las', '.0las'), ('sentir-ie', '.0ye'),
                                   ('partindo-ne', '.0ne'), ('contaz-me-la', '.0la'),
                                   ('fendo-le-ne', '.0ne'), ('da-me-ne', '.0ne'),
                                   ('fer-los', '.0los'), ("minchand-mo'ne", '`00 `[.1mo')):
            actual=phonemes(an, sentence)
            assert expected in actual, (sentence, actual)
            checks += 1
        assert '.1la' in phonemes(an, 'hola la')
        assert '.1mun' in phonemes(an, 'hola-mundo')
        checks += 2
        # Independent function words must not inherit Spanish abbreviation
        # expansion; they are weak in connected speech, but remain audible
        # and stressed when isolated or explicitly accented.
        for word in ('pa',):
            actual=phonemes(an, word)
            assert f'.1{word}' in actual and 'min' not in actual and 'por' not in actual, (word, actual)
            checks += 1
        for sentence, expected in (('pa casa', '.0pa'),
                                   ("d'o mundo", '.0do'), ('d’o Pirineu', '.0do'), ('ta casa', '.0ta'),
                                   ('enta casa', '`00 `[.1en.0ta'), ('bi ha', '.0bi'),
                                   ('ne veyo', '.0ne'), ('as casas', '.0as'),
                                   ('ro dia', '.0Ro'), ('ra casa', '.0Ra')):
            actual=phonemes(an, sentence)
            assert expected in actual, (sentence, actual)
            checks += 1
        assert '.1do' in phonemes(an, "d'o. Casa")
        assert '.1do' in phonemes(an, 'Dó ye?')
        assert '.1dos' in phonemes(an, 'dos libros')
        checks += 3
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
            (folder / 'ARAmain.dic').write_bytes(b'probexword\tcasa\nsinyal\tcasa\nyema\tllama\npa\tcasa\n')
            (folder / 'ESPmain.dic').write_bytes(b'probexword\tnaranja\n')
            before = render(an, ['probexword.', EciIndex(7)])
            an.set_dictionary_path(folder)
            es.set_dictionary_path(folder)
            for flag in (0, 1):
                an.set_parameter(DICTIONARY, flag)
                assert render(an, ['probexword.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(an, ['sinyal.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(an, ['yema.', EciIndex(7)]) == render(an, ['llama.', EciIndex(7)])
                assert render(an, ['pa.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(es, ['probexword.', EciIndex(7)]) == render(es, ['naranja.', EciIndex(7)])
                checks += 5
            an.set_dictionary_path(None)
            an.set_parameter(DICTIONARY, 0)
            assert render(an, ['probexword.', EciIndex(7)]) == before
            checks += 1
    finally:
        es.close(); an.close()
    print(f'Aragonese: {checks} native pronunciation, language, character, index and dictionary checks passed')


if __name__ == '__main__':
    main()
