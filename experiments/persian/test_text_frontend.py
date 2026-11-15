"""Exercise text analysis, word stress and safe bilingual alignment."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re

from espeak_ng_probe import EspeakPhonemeProbe
from text_frontend import (SpeechPart, UnsupportedPronunciation, native_tag_accepted,
                           parse_word, translate_text, word_tag, word_tags)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    parser.add_argument('--probe', type=Path, help='also validate tags in the private native probe')
    args = parser.parse_args()
    cases = {
        "s|a|l|'A|m": '`[.0sA.1lam]',
        "d|'o|n|j|A": '`[.1do.0nja]',
        "'i:|R|A|n": "`[.1'i:'.0r'a:'n]",
        "n|'o|q|1|t|e:": "`[.1noR.0t'e:']",
        "q|1|a|l|'a|m": '`[.0RA.1lAm]',
        "k|'A|q|1|a|z": '`[.1ka.0RAz]',
        "x|'A|n|e:": "`[.1Ga.0n'e:']",
        "p|a|n|dZ|'a|R|e:": "`[.0pAn.1JA.0r'e:']",
        "tS|e|t|'o:|R|i": "`[.0Ce.1t'o:'.0ri]",
        "s|,a:|d|o": "`[.2s'A:'.0do]",
        "s|'A|?|a|t": '`[.1sa.0wAt]',
        "d|'a|h": '`[.1dAhw]',
        "m|'e:|h|R": "`[.1m'e:'hr]",
        "b|'a|d|_|b|a|x|t": '`[.1bAd.0bAGt]',
        "t|a|?|'a|dZ|dZ|o|b": '`[.0tA.1wAJ.0Job]',
        "t|a|dZ|R|'o|b|e:": "`[.0tAJ.1ro.0b'e:']",
        "s|'o|_|A|l": '`[.1so]~`[.0al]',
        "b|,a|n|A:|_|b|'a|R|_|i|n": "`[.2bA.0n'a:'.1bAr]~`[.0in]",
        "t|a|?|_|'a|h|_|h|o|d": '`[.0tA.1wAh.0hod]',
        "m|'o|?|e|d:": '`[.1mo.0wedd]',
        "m|o|q|1|'a|?:|a|R": '`[.0mo.1RA.0wwAr]',
        "w|'i|n|d|o|w|s": '`[.1vin.0dovs]',
    }
    for source, expected in cases.items():
        assert word_tag(source) == expected, (source, word_tag(source), expected)
    for expected, displayed in (
        ('`[.1e.0hja]', '`[.1eh.0ja]'),
        ('`[.1mehr.0bod]', '`[.1mer.0bod]'),
        ('`[.1dAhw]', '`[.1dAw]'),
    ):
        assert native_tag_accepted(expected, displayed), (expected, displayed)
    for expected, displayed in (
        ('`[.1e.0hja]', '`[.0eh.1ja]'),
        ('`[.1mehr.0bod]', '`[.1meb.0bod]'),
        ('`[.1mehr.0bod]', '`[.1me.0bod]'),
    ):
        assert not native_tag_accepted(expected, displayed), (expected, displayed)
    assert word_tags(["_|'A|j|l|A|R"]) == ('`[.1aj.0lar]',)
    assert word_tags(["s|a|l|'A|m", "_|'A|j|l|A|R"]) == (
        '`[.0sA.1lam]', '~`[.1aj.0lar]')
    assert word_tags(["'a|d|A|_", "'A:|m|a|d"]) == (
        '`[.1A.0da]', "~`[.1'a:'.0mAd]")
    assert word_tags(["'a|d|A|_", "s|a|l|'A|m"]) == (
        '`[.1A.0da]', '`[.0sA.1lam]')
    checks = len(cases)
    for bad in ('s|?', 'x|@', 'q|2', 's||a', "s|q|'a", 's', "'k|a", 'a|h|?|a'):
        try:
            parse_word(bad)
        except UnsupportedPronunciation:
            checks += 1
        else:
            raise AssertionError(('unsupported phone accepted', bad))
    native = None
    if args.probe:
        from test_selected_probe import engine, set_probe
        set_probe(a=1, q=7, x=1, rise=2, h=5, l=4, vowel=1, s=1,
                  a_target='salam-fixed', final=1, glottal=1)
        native = engine(args.probe, 0x90000)
    with EspeakPhonemeProbe(args.library, args.data_parent) as probe:
        for source, expected in (
            ('سلام دنیا؟', (SpeechPart('fa', '`[.0sA.1lam] `[.1do.0nja]?'),)),
            ('ایران نقطه قلم کاغذ', (SpeechPart('fa',
             "`[.1'i:'.0r'a:'n] `[.1noR.0t'e:'] `[.0RA.1lAm] `[.1ka.0RAz]"),)),
            ('ساعت', (SpeechPart('fa', '`[.1sa.0wAt]'),)),
            ('ده ۱۲ مهر', (SpeechPart('fa', "`[.1dAhw] `[.0dA.1vaz.0dAhw] `[.1m'e:'hr]"),)),
            ('من NVDA را دوست دارم.', (SpeechPart('fa', '`[.0mAn]'),
             SpeechPart('en-us', 'NVDA'),
             SpeechPart('fa', "`[.0ra] `[.1d'u:'st] `[.1da.0rAm]."))),
            ('سلام Apple NVDA دنیا', (SpeechPart('fa', '`[.0sA.1lam]'),
             SpeechPart('en-us', 'Apple NVDA'), SpeechPart('fa', '`[.1do.0nja]'))),
        ):
            actual = translate_text(probe, source)
            assert actual == expected, (source, actual, expected)
            checks += 1
        assert translate_text(probe, 'salam donya?', pinglish=True) == (
            SpeechPart('fa', '`[.1sA.0lAm] `[.1do.0njA]?'),)
        assert translate_text(probe, 'salam', pinglish=False) == (SpeechPart('en-us', 'salam'),)
        assert translate_text(probe, 'salam father donya', pinglish=True) == (
            SpeechPart('fa', '`[.1sA.0lAm]'), SpeechPart('en-us', 'father'),
            SpeechPart('fa', '`[.1do.0njA]'))
        checks += 3
        for source in ('سلام https://example.com/abc دنیا',):
            try:
                translate_text(probe, source)
            except UnsupportedPronunciation:
                checks += 1
            else:
                raise AssertionError(('unimplemented case silently accepted', source))
        corpus = Path(__file__).with_name('reference_espeak_1_51.jsonl')
        mapped = 0
        unsupported = {}
        for line in corpus.read_text(encoding='utf8').splitlines():
            row = json.loads(line)
            if row['voice'] not in ('fa', 'fa-latn'):
                continue
            try:
                result = translate_text(probe, row['text'], pinglish=row['voice'] == 'fa-latn')
                assert result
                if native:
                    for part in result:
                        if part.language == 'fa':
                            for tag in re.findall(r'`\[[^]]+\]', part.text):
                                assert native_tag_accepted(tag, native._phonemes_for_probe(tag)), (
                                    'native tag changed or dropped phones', row['text'], tag,
                                    native._phonemes_for_probe(tag))
                mapped += 1
            except UnsupportedPronunciation as error:
                unsupported[str(error)] = unsupported.get(str(error), 0) + 1
        assert mapped >= 45, ('unexpected regression in corpus coverage', mapped, unsupported)
        print(f'Mapped {mapped} eSpeak reference texts; {sum(unsupported.values())} explicitly unsupported.')
        for error, number in sorted(unsupported.items()):
            print(f'  {number}: {error}')
    if native:
        native.close()
    print(f'Passed {checks} word, stress, punctuation, language-routing and rejection checks.')


if __name__ == '__main__':
    main()
