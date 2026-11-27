"""German wachsen/Gewächs pronunciations through the shared ECI path."""
import argparse
from pathlib import Path
import re
import tempfile

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciText, NATIVE_SPEED,
                               PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render

ROOT = Path(__file__).resolve().parents[1]


def engine(library, *, shipped=False):
    result = DirectEciEngine(library.resolve(), dialect=0x40000, encoding='utf8',
        native_voice=True, lexicon_path=ROOT / 'data/german_apple_observed.rlex' if shipped else None)
    if shipped:
        result.set_dictionary_path(ROOT / 'data/community')
    result.set_parameter(PROCESS_TAGS, 1)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference', type=Path)
    args = parser.parse_args()
    current = engine(args.library)
    old = engine(args.reference) if args.reference else None
    fixed = {'wächst': 'wäckst', 'Wächst': 'Wäckst', 'WÄCHST': 'WÄCkST',
             'überwächst': 'überwäckst', 'nachwächst': 'nachwäckst',
             'heranwächst': 'heranwäckst', 'Gewächs': 'Gewäcks',
             'Gewächshaus': 'Gewäckshaus', 'Pflanzengewächs': 'Pflanzengewäcks',
             'ihr wachst': 'ihr wackst', 'Ihr wachst': 'Ihr wackst',
             'Wachst ihr': 'Wackst ihr', 'IHR WACHST': 'IHR WACkST',
             'ihr nachwachst': 'ihr nachwackst',
             'Wenn ihr wachst': 'Wenn ihr wackst',
             'Ich finde, ihr wachst': 'Ich finde, ihr wackst',
             'Wachst ihr Fragezeichen?': 'Wackst ihr Fragezeichen?',
             '„ihr wachst“': '„ihr wackst“',
             '“ihr wachst”': '“ihr wackst”',
             '"ihr wachst"': '"ihr wackst"',
             "'ihr wachst'": "'ihr wackst'",
             "ihr 'wachst'": "ihr 'wackst'",
             '(ihr wachst)': '(ihr wackst)', '[ihr wachst]': '[ihr wackst]',
             'Klammer auf(ihr wachst)': 'Klammer auf(ihr wackst)',
             '«ihr wachst»': '«ihr wackst»',
             'Wenn „ihr wachst“ so steht.': 'Wenn „ihr wackst“ so steht.',
             'The context “ihr wachst” is distinguished from “du wachst”.':
                 'The context “ihr wackst” is distinguished from “du wachst”.'}
    unchanged = ('wachst', 'Wachstube', 'wachsen', 'Wachstum', 'Gewächse',
                 'wächsern', 'Wächter', 'nächste', 'nächtlich', 'lachst',
                 'Dachs', 'Luchs', 'Schieberegler', 'Leserichtung', 'du wachst', 'Wachst du?',
                 'wachst ihr Haus', 'wachst ihr Fragezeichen', 'Du wachst ihr Fragezeichen?',
                 'nichtihr wachst', "x'ihr wachst", '(du) wachst ihr Haus',
                 "Du 'wachst' ihr Haus",
                 'Du bewachst ihr Haus.', 'Du bewachst ihr Kind.', 'bewachst ihr Haus',
                 'bewachst ihr kleines Haus', 'Du bewachst ihr kleines Haus.')
    checks = 0
    try:
        for word, expected in fixed.items():
            actual = re.findall(r'`\[[^]]*\]', phonemes(current, word))
            wanted = re.findall(r'`\[[^]]*\]', phonemes(current, expected))
            assert actual == wanted and actual, (word, actual, wanted)
            checks += 1
        if old:
            for sentence in ('Die Pflanze wächst.', 'Wenn sie wächst, freut er sich.',
                             'Er überwächst das Gewächshaus.', 'Sie wächst, sie wächst.',
                             'Ihr wachst schnell.', 'Wachst ihr heute?'):
                before = re.sub(r'`vv\d+', '', phonemes(old, sentence)).replace('vEXs', 'vEks').replace('vAxst', 'vAkst')
                after = re.sub(r'`vv\d+', '', phonemes(current, sentence))
                before, after = ' '.join(before.split()), ' '.join(after.split())
                assert after == before, (sentence, before, after)
                checks += 1
            for sentence in (
                'Vor Runde Klammer auf(ihr wachst Runde Klammer zu).',
                'Wenn „ihr wachst“ so steht.',
                'The context “ihr wachst” is distinguished from “du wachst”.',
            ):
                before = re.sub(r'`vv\d+', '', phonemes(old, sentence)).replace('vAxst', 'vAkst', 1)
                after = re.sub(r'`vv\d+', '', phonemes(current, sentence))
                before, after = ' '.join(before.split()), ' '.join(after.split())
                assert after == before, (sentence, before, after)
                checks += 1
        for rate in (0, 1, 3, 6):
            for speed in (50, 90):
                for voice in (current, old):
                    if voice:
                        voice.set_parameter(SAMPLE_RATE, rate)
                        voice.set_parameter(NATIVE_SPEED, speed)
                for word, expected in fixed.items():
                    actual = render(current, [word, EciIndex(7)])
                    wanted = render(current, [expected, EciIndex(7)])
                    assert actual == wanted and actual[0] and actual[1] == [(7, len(actual[0]))], (rate, speed, word)
                    checks += 1
                if old:
                    for word in unchanged:
                        assert render(current, [word, EciIndex(7)]) == render(old, [word, EciIndex(7)]), (rate, speed, word)
                        checks += 1
        sentence = 'Die Pflanze wächst und das Gewächshaus wächst.'
        expected = render(current, [sentence, EciIndex(19)])
        for cut in range(1, len(sentence)):
            assert render(current, [sentence[:cut], sentence[cut:], EciIndex(19)]) == expected, cut
            checks += 1
        assert render(current, ['Sie wäc', EciIndex(1), 'hst.', EciIndex(2)]) == \
               render(current, ['Sie wäc', EciIndex(1), 'kst.', EciIndex(2)])
        assert render(current, ['`vfg40 `vbhz150 wächst `vbhz190 Gewächs']) == \
               render(current, ['`vfg40 `vbhz150 wäckst `vbhz190 Gewäcks'])
        assert render(current, ['ihr `vfg40 wachst']) == render(current, ['ihr `vfg40 wackst'])
        assert render(current, ['Du wachst nachts, ihr wachst schnell.']) == \
               render(current, ['Du wachst nachts, ihr wackst schnell.'])
        checks += 4
        if old:
            for tags in (0, 1):
                current.set_parameter(PROCESS_TAGS, tags)
                old.set_parameter(PROCESS_TAGS, tags)
                assert render(current, [EciText('wächst', True)]) == render(old, [EciText('wächst', True)]), tags
                checks += 1
            current.set_parameter(PROCESS_TAGS, 1)
            old.set_parameter(PROCESS_TAGS, 1)
        shipped = engine(args.library, shipped=True)
        try:
            for word, expected_word in fixed.items():
                assert render(shipped, [word]) == render(shipped, [expected_word]), word
                checks += 1
        finally:
            shipped.close()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / 'DEUmain.dic').write_bytes('wächst\tHallo\n'.encode('cp1252'))
            custom = engine(args.library)
            try:
                custom.set_dictionary_path(path)
                assert re.findall(r'`\[[^]]*\]', phonemes(custom, 'wächst')) == \
                       re.findall(r'`\[[^]]*\]', phonemes(custom, 'Hallo'))
                checks += 1
            finally:
                custom.close()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / 'DEURoot.dic').write_bytes('wächst\t`[.1vEXst]\nwachst\t`[.1vAxst]\n'.encode('cp1252'))
            custom = engine(args.library)
            reference = engine(args.reference) if args.reference else None
            try:
                custom.set_dictionary_path(path)
                if reference:
                    reference.set_dictionary_path(path)
                    for word in ('wächst', 'überwächst', 'ihr wachst', '„ihr wachst“',
                                 "'ihr wachst'", "ihr 'wachst'"):
                        assert render(custom, [word]) == render(reference, [word]), word
                        checks += 1
            finally:
                custom.close()
                if reference:
                    reference.close()
    finally:
        current.close()
        if old:
            old.close()
    print(f'German wachsen/Gewächs: {checks} pronunciation, PCM, index and dictionary checks passed')


if __name__ == '__main__':
    main()
