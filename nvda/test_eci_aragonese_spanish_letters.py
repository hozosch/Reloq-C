"""Aragonese and Mexican Spanish letter names through the native ECI API."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, PROCESS_TAGS, SAMPLE_RATE
from test_eci_dash_pause import character_phonemes
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    checks = 0
    for language, names in (
        (0x130000, {'g':'gue', 'j':'i larga', 'v':'ve baixa',
                    'w':'ve dople', 'x':'xe', 'y':'ye'}),
        (0x20001, {'v':'ve chica', 'w':'doble u'}),
        (0x20000, {'v':'uve', 'w':'doble uve'}),
    ):
        engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                 encoding='utf8', native_voice=True)
        try:
            for tags in (0, 1):
                engine.set_parameter(PROCESS_TAGS, tags)
                for letter, name in names.items():
                    for case in (letter, letter.upper()):
                        result = render(engine, [EciText(case, True), EciIndex(7)])
                        expected = render(engine, [name, EciIndex(7)])
                        actual = character_phonemes(engine, case)
                        expected_phones = phonemes(engine, name)
                        if language == 0x130000 and letter == 'j':
                            assert result[0] and [m[0] for m in result[1]] == [7]
                            assert '.0i' in actual and '.1lar.0ga' in actual, actual
                        else:
                            assert result == expected, (hex(language), tags, case, name, len(result[0]), len(expected[0]))
                            assert actual == expected_phones, (hex(language), tags, case, actual, expected_phones)
                        checks += 2
                if language == 0x130000:
                    assert phonemes(engine, 'v') == phonemes(engine, 've baixa')
                    assert '.1bay' in phonemes(engine, 'la v casa')
                    assert '.1bay' not in phonemes(engine, 'NVDA')
                    assert '.1bay' not in phonemes(engine, 'V B')
                    # A V next to other spelled letters uses short ve; the
                    # standalone name must not be inserted into the sequence.
                    short = character_phonemes(engine, 'VB')
                    assert '.1bai' not in short and '.1be' in short, short
                    whole = render(engine, [EciText('VB', True)])[0]
                    split, marks = render(engine, [EciText('V', True), EciIndex(12), EciText('B', True)])
                    assert whole == split and [m[0] for m in marks] == [12], marks
                    spaced = render(engine, [EciText('V', True), ' ', EciText('B', True)])[0]
                    assert spaced and spaced != render(engine, [EciText('V', True)])[0]
                    if not tags:
                        literal = phonemes(engine, '`ts2 Hola')
                        assert '.1te' in literal and '.1dos' in literal and '.1o.0la' in literal, literal
                        checks += 1
                    if tags:
                        assert phonemes(engine, '`ts2 V `ts0') == phonemes(engine, 've baixa')
                        assert '.1bay' not in phonemes(engine, '`ts2 VB `ts0')
                        checks += 2
                    checks += 6
            if language == 0x130000:
                engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    engine.set_parameter(SAMPLE_RATE, rate)
                    for prefix, softened in (('ve', True), ('la', True),
                                             ('camín', False), ('la,', False)):
                        actual, soft, stop = [len(render(engine, [f'{prefix} {word}'])[0])
                            for word in ('baixa', '`[.1Bay.0Sa]', '`[.1bay.0Sa]')]
                        assert soft != stop, (rate, prefix, soft, stop)
                        assert actual == (soft if softened else stop), (rate, prefix, actual, soft, stop)
                        checks += 1
                with tempfile.TemporaryDirectory() as directory:
                    folder = Path(directory)
                    (folder / 'ARAmain.dic').write_bytes(b'v\tcasa\nV\tnaranja\n')
                    engine.set_dictionary_path(folder)
                    for tags in (0, 1):
                        engine.set_parameter(PROCESS_TAGS, tags)
                        for letter, replacement in (('v', 'casa'), ('V', 'naranja')):
                            assert phonemes(engine, letter) == phonemes(engine, replacement)
                            assert render(engine, [letter]) == render(engine, [replacement])
                            assert character_phonemes(engine, letter) == phonemes(engine, 've baixa')
                            checks += 3
                    engine.set_dictionary_path(None)
            for rate in (1, 3, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                for letter in names:
                    assert render(engine, [EciText(letter, True)])[0], (hex(language), rate, letter)
                    checks += 1
            if args.baseline and language == 0x20000:
                previous = DirectEciEngine(args.baseline.resolve(), dialect=language,
                                           encoding='utf8', native_voice=True)
                try:
                    engine.set_parameter(SAMPLE_RATE, 1)
                    for text in ('Hola, mundo.', 'caixeta', 'Viva la música.', 've baixa'):
                        assert render(engine, [text]) == render(previous, [text]), text
                        checks += 1
                finally:
                    previous.close()
        finally:
            engine.close()
    print(f'Aragonese and Mexican Spanish: {checks} letter, spelling, rate and language-isolation checks passed')


if __name__ == '__main__':
    main()
