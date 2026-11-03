"""German native word, quantity, number, dictionary and index regressions."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render
from test_eci_german_overlay_case import lexicon


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                             encoding='utf8', experimental_generic=True)
    checks = 0
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        expected = {
            'famos': '`[.0fA.1mos]', 'rigoros': '`[.2ri.0go.1ros]',
            'famose': '`[.0fA.1mo.0z@]', 'rigorosen': '`[.2ri.0go.1ro.0z@n]',
            'Unterricht': '`[.1Un.0tR.2rIXt]', 'Untergang': '`[.1Un.0tR.2gAG]',
            'Überschrift': '`[.1y.0bR.2SrIft]', 'Übergabe': '`[.1y.0bR.2ga.0b@]',
            'Unterarm': '`[.1Un.0tR.2Arm]', 'Unterhose': '`[.1Un.0tR.2ho.0z@]',
            'Überlauf': "`[.1y.0bR.2l'aw'f]", 'Übermut': '`[.1y.0bR.2mut]',
            'Unterrichten': '`[.0Un.0tR.1rIX.0t@n]',
            'Unterlagen': '`[.0Un.0tR.1la.0g@n]',
            'Übernehmen': '`[.0y.0bR.1ne.0m@n]',
            'Überschriften': '`[.0y.0bR.1SrIf.0t@n]',
            '1 Test': "`[.1'aj'n] `2 `[.1tEst]",
            'plus': '`[.1plus]',
            'Dachausbau': "`[.1dak.2h'aw's.2b'aw']",
        }
        for word, required in expected.items():
            got = phonemes(engine, word)
            assert required in got, (word, got, required)
            checks += 1
        for rate in (1, 3, 2, 4, 5, 6):
            engine.set_parameter(SAMPLE_RATE, rate)
            for word in ('Infos', 'INfos', 'Demos', 'Windows', 'Fotos', 'famos', 'rigoros'):
                spr = phonemes(engine, word)
                assert render(engine, [word, EciIndex(7)]) == render(engine, [spr, EciIndex(7)]), (rate, word, spr)
                checks += 1
            for number in ('22', '23', '27', '29', '32', '33', '37', '39'):
                spr = phonemes(engine, number)
                assert render(engine, [number, EciIndex(7)]) == render(engine, [spr, EciIndex(7)]), (rate, number)
                checks += 1
            # In four-and compounds the consonantal /r/ has its own source
            # gesture. Text and explicit SPR must at least preserve the same
            # timeline, including the index at the end of the number.
            for number in ('24', '34', '44', 'vierund'):
                spr = phonemes(engine, number)
                text_audio, text_indices = render(engine, [number, EciIndex(7)])
                spr_audio, spr_indices = render(engine, [spr, EciIndex(7)])
                assert len(text_audio) == len(spr_audio) and text_indices == spr_indices, (rate, number)
                checks += 1
            for number in ('25', '26', '28'):
                spr = phonemes(engine, number)
                assert len(render(engine, [number])[0]) > len(render(engine, [spr])[0]), (rate, number)
                checks += 1
            for word in ('Pizza', 'Mama', 'Oma'):
                spr = phonemes(engine, word)
                assert len(render(engine, [word])[0]) > len(render(engine, [spr])[0]), (rate, word)
                checks += 1
        for word, expanded in (('Sprachauswahl', 'Sprach-Auswahl'),
                               ('Suchauswahl', 'Such-Auswahl'),
                               ('Sprachauswahlen', 'Sprach-Auswahlen'),
                               ('Suchauswahlen', 'Such-Auswahlen'),
                               ('Fachauswahl', 'Fach-Auswahl')):
            assert phonemes(engine, word) == phonemes(engine, expanded), word
            assert render(engine, [word, EciIndex(7)]) == render(engine, [expanded, EciIndex(7)]), word
            checks += 1
        for phrase in ('Pizza ist gut.', 'Pizza. Test.'):
            before = render(engine, [phrase, EciIndex(7)])
            render(engine, ['Pizza', EciIndex(7)])
            assert render(engine, [phrase, EciIndex(7)]) == before, phrase
            checks += 1
    finally:
        engine.close()
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        # The caller's Main replacement still wins over any built-in fix.
        (folder/'DEUmain.dic').write_bytes(b'Sprachauswahl\tKirsche\nSuchauswahl\tFenster\n')
        engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                                 encoding='utf8', experimental_generic=True)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_dictionary_path(folder)
            for word, replacement in (('Sprachauswahl', 'Kirsche'), ('Suchauswahl', 'Fenster')):
                assert render(engine, [word, EciIndex(7)]) == render(engine, [replacement, EciIndex(7)]), word
                checks += 1
        finally:
            engine.close()
        explicit = folder/'custom.rlex'
        lexicon(explicit, (('Sprachauswahl', '`[.1tEst]'),
                           ('Suchauswahl', '`[.1kAt]'),
                           ('Unterricht', '`[.0Un.0tR.1rIXt]'),
                           ('Pizza', '`[.1pI.0TA]')))
        engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                                 encoding='utf8', experimental_generic=True,
                                 lexicon_path=explicit)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            for word, expected in (('Sprachauswahl', '`[.1tEst]'),
                                   ('Suchauswahl', '`[.1kAt]'),
                                   ('Unterricht', '`[.0Un.0tR.1rIXt]')):
                assert expected in phonemes(engine, word), (word, phonemes(engine, word))
                checks += 1
            assert render(engine, ['Pizza', EciIndex(7)]) == render(engine, ['`[.1pI.0TA]', EciIndex(7)])
            checks += 1
        finally:
            engine.close()
    print(f'German rc16: {checks} native phoneme, long-o, rate, number, dictionary and index checks')


if __name__ == '__main__':
    main()
