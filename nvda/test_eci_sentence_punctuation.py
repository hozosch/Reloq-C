"""Sentence and comma boundaries must survive ordinary dictionary processing."""
import argparse
from pathlib import Path
import sys

from _reloq_eci_direct import DirectEciEngine, EciIndex, DICTIONARY
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    phrases = {
        0x10000: 'Something follows.', 0x10001: 'Something follows.',
        0x20000: 'Esto sigue.', 0x20001: 'Esto sigue.',
        0x30000: 'Bonjour tout le monde.', 0x30001: 'Bonjour tout le monde.',
        0x40000: 'Hallo zusammen.', 0x50000: 'Questa frase segue.',
        0x60000: '这是下一句。', 0x60001: '這是下一句。',
        0x70000: 'Esta frase continua.', 0x80000: 'これは次の文です。',
        0x90000: 'Tämä on seuraava lause.', 0xa0000: '다음 문장입니다.',
        0x120000: 'Αυτή είναι η επόμενη πρόταση.',
    }
    checks = 0
    for language, following in phrases.items():
        engine = DirectEciEngine(library, dialect=language, encoding='utf8', native_voice=True)
        try:
            for letter in ('X', 'b'):
                text = f'{letter}. {following}'
                separated = f'{letter}.  {following}'
                assert phonemes(engine, text) == phonemes(engine, separated), (hex(language), text)
                assert render(engine, [text])[0] == render(engine, [separated])[0], (hex(language), text)
                # Fragment boundaries and callback indices are kept in the
                # caller's original text, even where a space is inserted.
                whole = render(engine, [text, EciIndex(7)])
                split = render(engine, [text[:2], text[2:], EciIndex(7)])
                assert whole == split, (hex(language), text, 'fragment')
                checks += 3
        finally:
            engine.close()
    english = DirectEciEngine(library, dialect=0x10000, encoding='utf8', native_voice=True)
    try:
        for enabled in (0, 1):
            english.set_parameter(DICTIONARY, 0 if enabled else 1)
            with_commas = 'Mr., Ms., and MRS. Johnson are there.'
            without_commas = 'Mr. Ms. and MRS. Johnson are there.'
            spoken = phonemes(english, with_commas)
            assert spoken.count(',') >= 2, (enabled, spoken)
            assert len(render(english, [with_commas])[0]) > len(render(english, [without_commas])[0])
            checks += 2
    finally:
        english.close()
    print(f'Sentence punctuation: {checks} multilingual, fragment and abbreviation checks passed')


if __name__ == '__main__':
    main()
