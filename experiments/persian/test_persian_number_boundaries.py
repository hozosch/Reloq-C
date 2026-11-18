"""Reject Finnish spelling of adjacent native Persian phoneme controls."""
from __future__ import annotations

import argparse
from pathlib import Path

from direct_fa_sequence import EciIndex, EciText, project
from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import voice as make_voice
from text_frontend import native_sequence_accepted


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    # These are the actual Persian NVDA 2026.3.1 browse-mode announcements,
    # including the vowel mark immediately after the level number.
    examples = [f'سرنوشتارِ سطحِ {n}ِ {direction}‌ای نیست'
                for n in '123456' for direction in ('بعدی', 'قبلی')]
    examples += [f'سرنوشتارِ سطحِ {n}ِ بعدی‌ای نیست' for n in '۱۲۳۴۵۶']
    examples += ['1ِ', '12ِ', '1َ', '2ُ', 'سلام2', '2سلام', 'سلام 1ِ سلام']
    examples += ['1.5.0', '۱۲.۵.۰', '۱۲٫۵٫۰', 'سلام 1.5.0 خوب']
    frontend = DirectFirstPersian(args.library, None, None, None)
    voice, scope = make_voice(args.library, 0x90000)
    previous_scope = scope(1)
    try:
        for text in examples:
            analysis = frontend.analyze_direct(text)
            assert analysis and not analysis.used_fallback, text
            tag = analysis.parts[0].text
            assert ']`[' not in tag, (text, tag)
            shown = voice._phonemes_for_probe(tag)
            assert native_sequence_accepted(tag, shown), (text, tag, shown)
            assert len(analysis.source_map) == len(text.encode('utf8')) + 1
            assert all(a <= b for a, b in zip(analysis.source_map, analysis.source_map[1:])), text
        version = frontend.analyze_direct('1.5.0').parts[0].text
        assert version.count('noR') == 2 and version.count('`[') >= 5, version
        # An NVDA index between the number and vowel mark must preserve both
        # controls and produce the same sound text as an unsplit utterance.
        text = examples[0]
        index = text.index('1') + 1
        result = project((text[:index], EciIndex(49), text[index:]), frontend)
        assert result.tagged == frontend.analyze_direct(text).parts[0].text
        assert [part.value for part in result.parts if isinstance(part, EciIndex)] == [49]
        assert ''.join(part.text for part in result.parts if isinstance(part, EciText)) == result.tagged
        assert frontend._worker is None
    finally:
        frontend.close()
        scope(previous_scope)
        voice.close()
    print(f'Passed {len(examples)} Persian number/mark boundaries and native phoneme checks.')


if __name__ == '__main__':
    main()
