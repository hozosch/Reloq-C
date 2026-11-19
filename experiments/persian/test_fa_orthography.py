"""Check independent Persian rules for explicitly marked short vowels."""
from __future__ import annotations

import argparse
from pathlib import Path

from direct_fa_sequence import EciIndex, EciText, project
from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import voice
from text_frontend import native_sequence_accepted


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    frontend = DirectFirstPersian(args.library, None, None, None)
    engine, scope = voice(args.library, 0x90000)
    previous = scope(1)
    try:
        # The three written short vowels distinguish /pær/, /per/, /por/.
        # Persian's usual spelling omits these signs; never infer them from
        # the same unmarked consonants here.
        for text, expected in (('پَر', '`[.1pAr]'), ('پِر', '`[.1per]'),
                               ('پُر', '`[.1por]'), ('پَرْ', '`[.1pAr]'),
                               ('نَت', '`[.1nAt]'), ('نِت', '`[.1net]'),
                               ('نُت', '`[.1not]'), ('شُب', '`[.1Sob]')):
            analysis = frontend.analyze_direct(text)
            assert analysis and not analysis.used_fallback, text
            assert analysis.parts[0].text == expected, (text, analysis.parts)
            assert len(analysis.source_map) == len(text.encode('utf8')) + 1
            shown = engine._phonemes_for_probe(expected)
            assert native_sequence_accepted(expected, shown), (text, shown)
        # Explicit دَر keeps the existing weak preposition behavior before a
        # following word, but receives citation stress alone or at a pause.
        weak = frontend.analyze_direct('دَر خانه').parts[0].text
        assert weak.startswith('`00 `[.0dAr] '), weak
        assert frontend.analyze_direct('دَر').parts[0].text == '`[.1dAr]'
        assert frontend.analyze_direct('دَر، خانه').parts[0].text.startswith('`[.1dAr],')
        # Fully marked multisyllabic words now have a native rule. Still do
        # not guess an internal vowel that has not been marked, or shadda.
        marked_word=frontend.analyze_direct('پَرَپ')
        assert marked_word and marked_word.parts[0].text=='`[.0pA.1rAp]'
        assert frontend.analyze_direct('پَرپَپ') is None
        assert frontend.analyze_direct('پَرّ') is None
        marked = ('دَ', EciIndex(7), 'ر ', EciIndex(8), 'خانه')
        job = project(marked, frontend, allow_fallback=False)
        assert job.tagged == weak
        assert [part.value for part in job.parts if isinstance(part, EciIndex)] == [7, 8]
        assert ''.join(part.text for part in job.parts if isinstance(part, EciText)) == weak
        assert frontend._worker is None
    finally:
        frontend.close()
        scope(previous)
        engine.close()
    print('Passed independent Persian vowel marks, weak-word context and native index mapping.')


if __name__ == '__main__':
    main()
