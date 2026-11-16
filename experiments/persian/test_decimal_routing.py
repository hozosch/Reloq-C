"""Verify language selection for decimal numbers split into NVDA text pieces."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciCapitalWord, EciIndex, EciLanguage, EciText
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=option != 'worker')
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                  args.espeak_library, args.data_parent)
    def render(language, parts):
        engine.select_voice(language, '1')
        pcm, marks = bytearray(), []
        assert engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        return bytes(pcm), marks
    cases = (
        ('version 3.5 is available.',),
        ('version ', EciIndex(1), '3.5', EciIndex(2), ' available.'),
        ('version ', EciIndex(3), '3', EciIndex(4), '.', EciIndex(5), '5'),
        ('3.5 ', EciIndex(6), 'version'),
        (EciText('version ', segmentation_guarded=True), EciIndex(7), EciText('1,234.5')),
        (EciCapitalWord('NVDA', 'NV DA'), ' ', EciIndex(8), '1.5'),
        ('version ', EciIndex(9), '-3.5'),
        ('version ', EciIndex(10), '3', EciIndex(11), '.', EciIndex(12), '5', EciIndex(13), ' released'),
    )
    try:
        # The English decimal separator is a point; the English core says it
        # as "point", rather than merely matching two identically wrong paths.
        english = engine._activate(engine._key(US_ENGLISH.id, '1'))
        assert english._phonemes_for_probe('3.5') == english._phonemes_for_probe('three point five')
        for parts in cases:
            actual = render(PERSIAN_PROBE.id, parts)
            expected = render(US_ENGLISH.id, parts)
            assert actual == expected, (parts, len(actual[0]), len(expected[0]), actual[1], expected[1])
        # NVDA's Persian symbol processor normally turns the decimal point
        # into a Persian word. The preprocessor passes just the number as a
        # native text command, preserving the same English output and indexes.
        processed = ('version   ', EciText('3.5'), ' is available.  ')
        assert render(PERSIAN_PROBE.id, processed) == render(US_ENGLISH.id, ('version   3.5 is available.  ',))
        processed_split = ('version   ', EciText('3'), EciIndex(31), EciText('.'),
                           EciIndex(32), EciText('5'), ' available.  ')
        assert render(PERSIAN_PROBE.id, processed_split) == render(US_ENGLISH.id, processed_split)
        signed = ('version   ', EciText(' minus '), EciText('3.5'), ' is available.  ')
        assert render(PERSIAN_PROBE.id, signed) == render(US_ENGLISH.id, ('version    minus 3.5 is available.  ',))
        # A number on its own or next to Persian remains Persian, including
        # when the number was delivered in several indexed fragments.
        assert render(PERSIAN_PROBE.id, ('3.5',))[0] != render(US_ENGLISH.id, ('3.5',))[0]
        assert render(PERSIAN_PROBE.id, ('سلام ', '3.5'))[0] == render(PERSIAN_PROBE.id, ('سلام 3.5',))[0]
        explicit = ('version ', EciLanguage(PERSIAN_PROBE.id, '1'), '3.5')
        assert render(PERSIAN_PROBE.id, explicit)[0] != render(US_ENGLISH.id, ('version 3.5',))[0]
        for parts in (('phone 3.5',), ('phone ', EciIndex(20), '3', EciIndex(21), '.', '5'),
                      ('3.5 ', EciIndex(22), 'phone'), ('phone ', EciIndex(23), '-1.5')):
            assert render(PINGLISH_PROBE.id, parts) == render(US_ENGLISH.id, parts), parts
        assert render(PINGLISH_PROBE.id, ('salam 3.5',))[0] != render(US_ENGLISH.id, ('salam 3.5',))[0]
        assert render(PERSIAN_PROBE.id, ('version ', EciText('3.5', character_mode=True)))[0]
    finally:
        engine.close()
    print('Passed English and Pinglish decimal routing, source indexes, and Persian-language boundaries.')


if __name__ == '__main__':
    main()
