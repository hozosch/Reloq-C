"""Native Aragonese rules, language isolation, dictionaries and indices."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, DICTIONARY, PROCESS_TAGS, SAMPLE_RATE
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
        # In this voice tz is the Castilian-like interdental, including the
        # numerals, derived words and word-final plurals and verb forms.
        # Preserve t across a word boundary, and preserve Castilian rules.
        for written, reference in (('dotze', 'doze'), ('tretze', 'treze'),
                                   ('setze', 'seze'), ('dotzena', 'dozena'),
                                   ('mocetz', 'mocez'), ('fetz', 'fez'),
                                   ('verdatz', 'verdaz'), ('puetz', 'puez'),
                                   ('DOTZE', 'DOZE'), ('Dotze', 'Doze')):
            actual, expected = phonemes(an, written), phonemes(an, reference)
            assert actual == expected, (written, reference, actual, expected)
            checks += 1
        assert phonemes(es, 'dotze') != phonemes(es, 'doze')
        assert phonemes(an, 'gat zorro') != phonemes(an, 'ga zorro')
        checks += 2
        for number, reference in (('12', 'doze'), ('13', 'treze')):
            assert phonemes(an, number) == phonemes(an, reference), number
            checks += 1
        whole, _ = render(an, ['dotze libros y tretze casas.'])
        split, indices = render(an, ['dot', EciIndex(12), 'ze libros y tretze casas.'])
        assert whole == split and [i for i, _ in indices] == [12], indices
        checks += 1
        # Written -nt/-st/-rt is latent in Aragonese at the word end. Keep
        # the original stress, final /s/ in plurals and the following word.
        for word, expected in (('puent', '[.1pwen]'), ('chent', '[.1Cen]'),
                               ('dolent', '[.0do.1len]'), ('rebost', '[.0Re.1bos]'),
                               ('part', '[.1par]'), ('malament', '[.0ma.0la.1men]'),
                               ('chents', '[.1Cens]'), ('dolents', '[.0do.1lens]'),
                               ('málament', '[.1ma.0la.0men]'),
                               ('muit', '[.1mwi]'), ('molt', '[.1mol]'),
                               ('feit', '[.1fey]')):
            actual=phonemes(an, word)
            assert expected in actual, (word, actual)
            checks += 1
        for word, expected in (('mocet', '[.0mo.1Tet]'), ('puentes', '[.1pwen.0tes]'),
                               ('muito', '[.1mwi.0to]'),
                               ('siet', '[.1syet]')):
            actual=phonemes(an, word)
            assert expected in actual, (word, actual)
            checks += 1
        connected=phonemes(an, 'chent y malament, part. Casa.')
        assert '[.1Cen]' in connected and '[.0ma.0la.1men]' in connected and '[.1par]' in connected and '[.1ka.0sa]' in connected, connected
        assert '[.1pwent]' in phonemes(es, 'puent')
        checks += 2
        whole, _ = render(an, ['chent y casa.'])
        split, indices = render(an, ['chen', EciIndex(9), 't y casa.'])
        assert whole == split and [i for i,_ in indices] == [9], indices
        checks += 1
        # Number words are chosen inside the existing numeric and character
        # rules, including hundreds and the combined twenties.
        for number, word in (('7', 'siet'), ('8', 'ueito'), ('9', 'nueu'), ('12', 'dotze'),
                             ('13', 'tretze'), ('16', 'deciséis'), ('17', 'decisiet'),
                             ('18', 'deciueito'), ('19', 'decinueu'), ('27', 'vintisiet'),
                             ('20', 'vinte'), ('28', 'vintiueito'), ('29', 'vintinueu'),
                             ('30', 'trenta'), ('40', 'cuaranta'), ('50', 'cincuanta'),
                             ('60', 'sisanta'), ('70', 'setanta'), ('80', 'uitanta'),
                             ('90', 'novanta'), ('200', 'docientos'), ('300', 'trecientos'),
                             ('500', 'cincocientos'), ('600', 'seicientos'),
                             ('700', 'sietecientos'),
                             ('800', 'ueitocientos'), ('900', 'nueucientos')):
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
        for word in ('pa', 'do'):
            actual=phonemes(an, word)
            assert f'.1{word}' in actual and 'min' not in actual and 'por' not in actual, (word, actual)
            checks += 1
        for sentence, expected in (('pa casa', '.0pa'), ('a casa do vivo', '.0do'),
                                   ("d'o mundo", '.0do'), ('d’o Pirineu', '.0do'),
                                   ("d'a casa", '.0da'), ("d'os paises", '.0dos'),
                                   ('d’as casas', '.0das'), ('ta casa', '.0ta'),
                                   ('enta casa', '`00 `[.1en.0ta'), ('bi ha', '.0bi'),
                                   ('ne veyo', '.0ne'), ('as casas', '.0as'),
                                   ('ro dia', '.0Ro'), ('ra casa', '.0Ra')):
            actual=phonemes(an, sentence)
            assert expected in actual, (sentence, actual)
            checks += 1
        assert '.1do' in phonemes(an, "d'o. Casa")
        assert '.1pa' in phonemes(an, 'pa. Casa')
        assert '.1da' in phonemes(an, 'da casa')
        assert '.1do' in phonemes(an, 'Dó ye?')
        assert '.1dos' in phonemes(an, 'dos libros')
        assert '.1das' in phonemes(an, 'das casas')
        # Keep the weak word and following word intact across contexts where
        # the initial dental may become an approximant. A pause remains a
        # boundary, and a nasal at the end of the preceding word is allowed.
        for sentence in ("la casa d'o mundo", "as casas d'o mundo",
                         "lo camín d'o mundo", "la casa, d'o mundo"):
            actual=phonemes(an, sentence)
            assert '.0do' in actual and '.1mun' in actual, (sentence, actual)
            checks += 1
        for sentence in ('la casa de mundo', 'as casas de mundo', 'mar de mundo',
                         'lo camín de mundo', 'la casa, de mundo',
                         'signal de numero', 'sinyal de numero'):
            actual=phonemes(an, sentence)
            assert '.0de' in actual and ('.1mun' in actual or '.1me' in actual), (sentence, actual)
            checks += 1
        # The ECI phoneme text normalizes both dental allophones to 'd'.
        # Compare speech durations with explicit native D/d references so a
        # phoneme-only test cannot pass when the allophone is missing.
        old_tags=an._dll.eciGetParam(an._handle, PROCESS_TAGS)
        old_rate=an._dll.eciGetParam(an._handle, SAMPLE_RATE)
        try:
            an.set_parameter(PROCESS_TAGS, 1)
            for rate in (1, 5):
                an.set_parameter(SAMPLE_RATE, rate)
                for prefix, expected in (('la casa', 'soft'), ('as casas', 'soft'),
                                         ('mar', 'soft'), ('signal', 'soft'),
                                         ('sinyal', 'soft'), ('lo camín', 'stop'),
                                         ('la casa,', 'stop'), ('signal,', 'stop')):
                    for word, vowel in (("d'o", 'o'), ('de', 'e')):
                        actual, soft, stop = [len(render(an, [f'{prefix} {item} mundo'])[0])
                            for item in (word, f'`[.0D{vowel}]', f'`[.0d{vowel}]')]
                        assert soft != stop, (rate, prefix, word, soft, stop)
                        if expected == 'stop':
                            assert actual == stop, (rate, prefix, word, actual, soft, stop)
                        elif prefix == 'la casa':
                            assert actual == soft, (rate, prefix, word, actual, soft, stop)
                        else:
                            assert actual < stop, (rate, prefix, word, actual, soft, stop)
                        checks += 1
                for prefix in ('signal', 'sinyal'):
                    actual, stop = [len(render(an, [f'{prefix} {word} numero'])[0])
                                    for word in ('de', '`[.0de]')]
                    assert actual < stop, (rate, prefix, actual, stop)
                    checks += 1
                assert render(an, ['final dia'])[0] == render(an, ['final `[.1dya]'])[0]
                checks += 1
        finally:
            an.set_parameter(SAMPLE_RATE, old_rate)
            an.set_parameter(PROCESS_TAGS, old_tags)
        for isolated, expected in (("d'o", '.1do'), ("d'a", '.1da'),
                                   ("d'os", '.1dos'), ("d'as", '.1das')):
            actual = phonemes(an, isolated)
            assert expected in actual, (isolated, actual)
            checks += 1
        for sentence in ("d'os libros y dos amigos", "dos libros y d'os amigos"):
            actual = phonemes(an, sentence)
            assert '.0dos' in actual and '.1dos' in actual, (sentence, actual)
            checks += 1
        actual = phonemes(an, ' '.join(["d'os libros"] * 32) + ' y dos amigos')
        assert actual.count('.0dos') == 32 and actual.count('.1dos') == 1, actual
        checks += 1
        checks += 6
        whole, _ = render(an, ["d'os paises."])
        split, indices = render(an, ["d'", EciIndex(11), 'os paises.'])
        assert whole == split and [i for i,_ in indices] == [11], indices
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
            (folder / 'ARAmain.dic').write_bytes(b'probexword\tcasa\nsinyal\tcasa\nyema\tllama\npa\tcasa\ndo\tcasa\nchent\tcasa\n')
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
                assert render(an, ['do.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(an, ['chent.', EciIndex(7)]) == render(an, ['casa.', EciIndex(7)])
                assert render(es, ['probexword.', EciIndex(7)]) == render(es, ['naranja.', EciIndex(7)])
                checks += 7
            an.set_dictionary_path(None)
            an.set_parameter(DICTIONARY, 0)
            assert render(an, ['probexword.', EciIndex(7)]) == before
            checks += 1
    finally:
        es.close(); an.close()
    print(f'Aragonese: {checks} native pronunciation, language, character, index and dictionary checks passed')


if __name__ == '__main__':
    main()
