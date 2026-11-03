"""German dictionary compounds, connected nine, phrase-final A and R."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, NATIVE_PITCH,
    NATIVE_INFLECTION, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES, DICTIONARY)
from test_eci_german_clock_native import phonemes, render
from test_eci_german_overlay_case import lexicon


def engine(path, **kwargs):
    result = DirectEciEngine(path.resolve(), dialect=0x40000, encoding='utf8',
                             experimental_generic=True, **kwargs)
    result.set_parameter(PROCESS_TAGS, 1)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    checks = 0
    e = engine(args.library)
    old = engine(args.baseline) if args.baseline else None
    try:
        for gender in (0, 1):
            e.set_parameter(GENDER, gender)
            for rate in SAMPLE_RATES:
                e.set_parameter(SAMPLE_RATE, rate)
                for tail in ('. Test', '? Test', '! Test', '. Mama.'):
                    one = render(e, ['Pizza', EciIndex(1), '.'])[1][0][1]
                    two = render(e, ['Pizza', EciIndex(1), tail])[1][0][1]
                    assert one == two, (gender, rate, tail)
                    checks += 1
                for word in ('Rot', 'Raten', 'brennen', 'vierund', '29', '39', '99'):
                    audio, indices = render(e, [word, EciIndex(7)])
                    assert audio and indices == [(7, len(audio))], (gender, rate, word)
                    checks += 1
                for pitch in (65, 98):
                    e.set_parameter(NATIVE_PITCH, pitch)
                    e.set_parameter(NATIVE_INFLECTION, 30)
                    a, index = render(e, ['Der Task wird mit Administratorberechtigungen erstellt.', EciIndex(4)])
                    assert a and index == [(4, len(a))]
                    checks += 1
                for question in ('Wie geht das?', 'Warum funktioniert das nicht?',
                                 'Möchtest du Kaffee oder Tee?', 'Was?'):
                    audio, index = render(e, [question, EciIndex(8)])
                    assert audio and index == [(8, len(audio))], (gender, rate, question)
                    checks += 1
                e.set_parameter(NATIVE_PITCH, 65)
        if old:
            for item in (e, old):
                item.set_parameter(GENDER, 0)
                item.set_parameter(SAMPLE_RATE, 1)
                item.set_parameter(NATIVE_PITCH, 65)
            assert len(render(e, ['neunund'])[0]) < len(render(old, ['neunund'])[0]); checks += 1
            for word in ('neun', 'neun und', 'siebenund', '29', '99'):
                assert render(e, [word]) == render(old, [word]), word
                checks += 1
            assert render(e, ['Rot']) != render(old, ['Rot']); checks += 1
    finally:
        e.close()
        if old: old.close()

    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        (root / 'DEURoot.dic').write_bytes(
            b'eloquence\t`[.1E.0lO.2kvEns]\nread\t`[.1rid]\n')
        (root / 'DEUmain.dic').write_bytes(b'Solo\t`[.1tEst]\nTokenX\tbei\n')
        custom = root / 'user.rlex'
        lexicon(custom, (('Zup', '`[.1tEst]'), ('zup', '`[.1kAt]')))
        e, plain = engine(args.library, lexicon_path=custom), engine(args.library)
        try:
            e.set_dictionary_path(root)
            for word in ('ETI-Eloquence', 'Test-Eloquence',
                         'Eloquence-Test', 'ETI-Eloquence-Ausgabe', 'NVDA-Eloquence',
                         'NVDA-A-Eloquence', 'ETI-A-Eloquence'):
                p = phonemes(e, word)
                assert 'kvEns' in p and 'kvEn.0T@' not in p and 'klA.0mR' not in p, (word, p)
                assert p != phonemes(plain, word), word
                checks += 1
            # Equivalent quote pairs must not split the compound or expose
            # generated phonetic markup as spoken text.
            for word in ('ETI-Eloquence', 'NVDA-Eloquence', 'NVDA-A-Eloquence',
                         'Sprach-Auswahl', 'Wort-Analyse'):
                quoted = f'"{word}"'
                expected = render(e, ['Das Wort ', EciIndex(1), quoted, EciIndex(2), ' ist gut.', EciIndex(3)])
                for left, right in (('„', '“'), ('“', '”'), ('«', '»'), ('»', '«'), ("'", "'"), ('‚', '‘')):
                    actual = render(e, ['Das Wort ', EciIndex(1), left + word + right, EciIndex(2), ' ist gut.', EciIndex(3)])
                    assert actual == expected, (word, left, right)
                    checks += 1
                assert render(e, [word, EciIndex(7)]) == render(e, [phonemes(e, word), EciIndex(7)]), word
                checks += 1
            for passage in ('das rote ETI-Eloquence Ding',
                            'ETI-Eloquence ist gut.',
                            'Sprach-Auswahl und Wort-Analyse'):
                expected = render(e, ['Er sagte: "' + passage + '" und ging.'])
                for left, right in (('„', '“'), ('“', '”'), ('«', '»'), ('‚', '‘')):
                    assert render(e, ['Er sagte: ' + left + passage + right + ' und ging.']) == expected, (passage, left)
                    checks += 1
            # A question mark immediately before a typographic closing quote
            # must retain sentence punctuation rather than become spoken text.
            for question in ('Wie geht das?', 'Warum funktioniert das nicht?', 'Was?'):
                expected = render(e, ['Er sagte: "' + question + '"'])
                expected_phones = phonemes(e, '"' + question + '"')
                for left, right in (('„', '“'), ('“', '”'), ('«', '»'), ('‚', '‘')):
                    quoted = left + question + right
                    assert phonemes(e, quoted) == expected_phones, quoted
                    assert render(e, ['Er sagte: ' + quoted]) == expected, quoted
                    checks += 1
            p = phonemes(e, 'Test-Solo')
            assert 'bIn.0d@' not in p and p.count('tEst') == 2, p
            checks += 1
            for word, target in (('ETI-Zup', 'tEst'), ('ETI-zup', 'kAt')):
                assert target in phonemes(e, word), word
                checks += 1
            for word in ('ETI-Eloquence', 'ETI-Eloquence-Ausgabe', 'NVDA-Eloquence'):
                whole = render(e, ['Die ', EciIndex(1), word, EciIndex(2), ' ist gut.', EciIndex(3)])
                split = word.find('-') + 1
                parts = render(e, ['Die ', EciIndex(1), word[:split], word[split:], EciIndex(2), ' ist gut.', EciIndex(3)])
                assert whole == parts, (word, whole[1], parts[1])
                checks += 1
            for word in ('ETI-Eloquence', 'Test-Eloquence'):
                assert phonemes(e, '`da0 ' + word) != phonemes(plain, '`da0 ' + word), word
                assert phonemes(e, '`da1 ' + word) != phonemes(plain, '`da1 ' + word), word
                checks += 2
            # If the native compound already applies the user's constituent,
            # replacing it with isolated SPR can incorrectly move the phrase
            # focus from the final adjective or noun onto the acronym.
            for tags in (0, 1):
                e.set_parameter(PROCESS_TAGS, tags)
                for text,ending in (
                    ('Die NVDA-Eloquence-Einstellungen sind korrekt.', "`[.0kO.1rEkt]"),
                    ('Die Eloquence-C-Variante liest den Text.', "`[.1tEkst]"),
                    ('„Die NVDA-Eloquence-Einstellungen sind korrekt.“', "`[.0kO.1rEkt]"),
                ):
                    p = phonemes(e, text)
                    assert 'kvEns' in p, (text, p)
                    assert p.rsplit('`2 ', 1)[-1].startswith(ending), (text, p)
                    audio, marks = render(e, [text, EciIndex(7)])
                    assert audio and marks == [(7, len(audio))], (text, marks)
                    checks += 1
            e.set_parameter(PROCESS_TAGS, 1)
            e.set_parameter(DICTIONARY, 1)
            plain.set_parameter(DICTIONARY, 1)
            assert render(e, ['ETI-Eloquence']) != render(plain, ['ETI-Eloquence']); checks += 1
        finally:
            e.close()
            plain.close()
    print(f'German rc18: {checks} rate/model, R, A, nine and dictionary compound checks')


if __name__ == '__main__':
    main()
