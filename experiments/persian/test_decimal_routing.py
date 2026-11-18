"""Verify language selection for decimal numbers split into NVDA text pieces."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciCapitalWord, EciIndex, EciLanguage, EciText
from direct_fa_sequence import project
from nvda_driver_probe import (PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine,
                               _persian_part_runs, _pinglish_part_runs)
from text_frontend import native_sequence_accepted


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
        ('Google Chrome, version 157.0.8103.79 (AMD64)',),
        ('BlindRadio, version 0.2.100',),
        ('v1.5.0',),
        ('NVDA1.5.0',),
        ('4 of 14',),
        ('4', EciIndex(29), ' of ', EciIndex(30), '14'),
        ('ˌchi-kən-ˈgu̇n-yə',),
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
        # With NVDA's builtin rule, the prefix is processed as ordinary text
        # and the decimal is a native command beginning with its inserted
        # separator. The complete result must still be the English version.
        version = ('v  ', EciText(' 1.5.0'))
        assert render(PERSIAN_PROBE.id, version) == render(US_ENGLISH.id, ('v 1.5.0  ',))
        version_indexed = ('v  ', EciText(' 1'), EciIndex(33), EciText('.5.0'))
        assert render(PERSIAN_PROBE.id, version_indexed) == render(US_ENGLISH.id, version_indexed)
        mixed = ('سلام v  ', EciText(' 1.5.0'), ' خوب')
        runs = _persian_part_runs(mixed, (PERSIAN_PROBE.id, '1'))
        assert runs[1] == (('en', ' 1.5.0'),), runs
        initialism_version = ('NVDA  ', EciText(' 1.5.0'))
        assert render(PERSIAN_PROBE.id, initialism_version) == render(US_ENGLISH.id, initialism_version)
        assert render(PERSIAN_PROBE.id, initialism_version) == render(US_ENGLISH.id, ('NVDA1.5.0',))
        # A number on its own or next to Persian remains Persian, including
        # when the number was delivered in several indexed fragments.
        assert render(PERSIAN_PROBE.id, ('3.5',))[0] != render(US_ENGLISH.id, ('3.5',))[0]
        assert render(PERSIAN_PROBE.id, ('سلام ', '3.5'))[0] == render(PERSIAN_PROBE.id, ('سلام 3.5',))[0]
        assert render(PERSIAN_PROBE.id, ('سلام 4 of 14',))[0]
        explicit = ('version ', EciLanguage(PERSIAN_PROBE.id, '1'), '3.5')
        assert render(PERSIAN_PROBE.id, explicit)[0] != render(US_ENGLISH.id, ('version 3.5',))[0]
        for parts in (('phone 3.5',), ('phone ', EciIndex(20), '3', EciIndex(21), '.', '5'),
                      ('3.5 ', EciIndex(22), 'phone'), ('phone ', EciIndex(23), '-1.5'),
                      ('v1.5.0',), ('NVDA1.5.0',)):
            runs = _pinglish_part_runs(parts, (PINGLISH_PROBE.id, '1'))
            assert all(not english or not any(char.isdigit() for char in text)
                       for spans in runs.values() for english, text in spans), (parts, runs)
            assert render(PINGLISH_PROBE.id, parts)[0] != render(US_ENGLISH.id, parts)[0], parts
        pinglish_engine = engine._engines[PINGLISH_PROBE.id]
        pinglish_frontend = pinglish_engine._frontend
        scope = pinglish_engine._scope(1)
        try:
            for text in ('v1.5.0', 'v 1.5.0', 'NVDA1.5.0', 'NVDA 1.5.0', '1.5.0v'):
                job = project((text,), pinglish_frontend, pinglish=True, allow_fallback=False)
                shown = pinglish_engine._voice._phonemes_for_probe(job.tagged)
                assert job.tagged.count('noR') == 2 and not job.needs_native_validation, (text, job.tagged)
                assert native_sequence_accepted(job.tagged, shown), (text, job.tagged, shown)
        finally:
            pinglish_engine._scope(scope)
        split_job = project(('v 1', EciIndex(35), '.', EciIndex(36), '5.0'),
                            pinglish_frontend, pinglish=True, allow_fallback=False)
        assert split_job.tagged.count('noR') == 2
        assert [part.value for part in split_job.parts if isinstance(part, EciIndex)] == [35, 36]
        assert render(PINGLISH_PROBE.id, ('v 1.5.0',))[0] == render(
            PINGLISH_PROBE.id, ('v 1', EciIndex(35), '.', EciIndex(36), '5.0'))[0]
        assert render(PINGLISH_PROBE.id, ('salam 3.5',))[0] != render(US_ENGLISH.id, ('salam 3.5',))[0]
        assert render(PERSIAN_PROBE.id, ('version ', EciText('3.5', character_mode=True)))[0]
    finally:
        engine.close()
    print('Passed English and Pinglish decimal routing, source indexes, and Persian-language boundaries.')


if __name__ == '__main__':
    main()
