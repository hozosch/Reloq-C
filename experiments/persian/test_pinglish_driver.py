"""Check explicit Pinglish analysis, positions, names and native playback."""
from __future__ import annotations

import argparse
from pathlib import Path
import string

from direct_fa_sequence import EciIndex, EciText, ScopedDirectPersianEngine, project
from hybrid_fa_frontend import DirectFirstPersian
from pinglish_frontend import LATIN_NAMES, normalize_pinglish
from text_frontend import UnsupportedPronunciation, word_tag


def check_mapping(text, result):
    assert result is not None and result.source_map is not None, text
    assert len(result.source_map) == len(text.encode('utf8')) + 1
    assert result.source_map[-1] == len(result.parts[0].text)
    assert all(a <= b for a, b in zip(result.source_map, result.source_map[1:]))


def analysis_tests(frontend):
    pairs = {'salam donya?': 'سلام دنیا?', 'Salaam Donyaa!': 'سلام دنیا!',
             'salam ۱۲ donya?': 'سلام ۱۲ دنیا?'}
    for source, expected in pairs.items():
        normalized, mapping = normalize_pinglish(source)
        assert normalized == expected
        assert len(mapping) == len(source.encode('utf8')) + 1
        assert mapping[-1] == len(normalized.encode('utf8'))
        assert all(a <= b for a, b in zip(mapping, mapping[1:]))
        result = frontend.analyze_pinglish(source, positioned=True, allow_fallback=True)
        assert result is not None and result.parts == frontend.analyze_direct(expected).parts
        assert not result.used_fallback
        check_mapping(source, result)
    for text in ('SALAM', 'salam2', 'xsalam', 'salam\u200c', 'salam\u0301', "salam's"):
        assert normalize_pinglish(text)[0] == text
    for parts in ((), (EciIndex(1),), (' ', EciIndex(2))):
        job = project(parts, frontend, pinglish=True, allow_fallback=True)
        assert not job.tagged.strip()
    parts = ('sa', EciIndex(4), 'lam ', EciIndex(5), 'do', EciIndex(6), 'nya?')
    job = project(parts, frontend, pinglish=True, allow_fallback=True)
    assert job.tagged == frontend.analyze_direct('سلام دنیا?').parts[0].text
    assert [part.value for part in job.parts if isinstance(part, EciIndex)] == [4, 5, 6]
    assert isinstance(job.parts[0], EciIndex) and job.parts[0].value == 4
    assert not job.needs_native_validation
    for char in string.ascii_letters:
        name = frontend.analyze_pinglish_character(char)
        assert name is not None and name.parts[0].text == LATIN_NAMES[char.lower()]
        job = project((EciText(char, character_mode=True),), frontend, pinglish=True)
        assert job.tagged == name.parts[0].text and not job.needs_native_validation
    assert frontend.analyze_pinglish_character('ق') == frontend.analyze_character('ق')
    assert frontend._worker is None

    worker = frontend._fallback()
    for char in string.ascii_letters:
        rows = worker.translate('\x0118Y' + char + '\x01Y', voice='fa-latn', separator='|')
        words = [word for row in rows for word in row.phonemes.split()]
        assert len(words) == 1 and words[0].endswith('|_!'), (char, words)
        assert word_tag(words[0][:-3]) == LATIN_NAMES[char.lower()], (char, words)

    for source in ('salam khub?', 'khoob salam', 'yek', 'salam, khub.',
                   'salam 12 khub', 'khoob ۳٫۱۴ donya?', 'khoob 12 khub 34?'):
        result = frontend.analyze_pinglish(source, positioned=True, allow_fallback=True)
        if result is None:
            normalized, _ = normalize_pinglish(source)
            print('Pinglish alignment diagnostic:', repr(normalized),
                  worker.translate(normalized.rstrip('?!. ,'), voice='fa-latn', separator='|'),
                  worker.events(normalized.rstrip('?!. ,'), voice='fa-latn'), flush=True)
        check_mapping(source, result)
        assert result.used_fallback
        assert result.parts == frontend.analyze_pinglish(source, positioned=False, allow_fallback=True).parts
        assert not frontend.analyze_pinglish(source, positioned=False, allow_fallback=False)
    word = frontend.analyze_pinglish('salam khub', positioned=True, allow_fallback=True)
    assert word.parts[0].text.startswith(frontend.analyze_direct('سلام').parts[0].text)
    assert word.source_map[len('salam ')] == word.parts[0].text.index('`[', 1)
    parts = ('salam ', EciIndex(7), 'khub?', EciIndex(8))
    job = project(parts, frontend, pinglish=True, allow_fallback=True)
    assert job.needs_native_validation and [p.value for p in job.parts if isinstance(p, EciIndex)] == [7, 8]
    assert frontend.analyze_pinglish('salam father donya', positioned=False, allow_fallback=True) is None
    numbered = project(('salam ', EciIndex(9), '1', EciIndex(10), '2 khub'),
                       frontend, pinglish=True, allow_fallback=True)
    assert numbered.needs_native_validation
    assert [p.value for p in numbered.parts if isinstance(p, EciIndex)] == [9, 10]
    for parts in (('`vp80 salam',), ('salam father donya',)):
        try:
            project(parts, frontend, pinglish=True, allow_fallback=True)
            raise AssertionError(f'unsupported input was accepted: {parts!r}')
        except UnsupportedPronunciation:
            pass


def playback_tests(frontend, probe):
    voice = ScopedDirectPersianEngine(probe, frontend, allow_fallback=True, pinglish=True)
    fa = ScopedDirectPersianEngine(probe, frontend, allow_fallback=True)
    try:
        for roman, script in (('salam donya?', 'سلام دنیا؟'), ('Salaam Donyaa!', 'سلام دنیا!')):
            expected, actual = bytearray(), bytearray()
            assert fa.speak((script,), expected.extend, lambda _: None)
            assert voice.speak((roman,), actual.extend, lambda _: None)
            assert expected and actual == expected
        for parts in ((EciText('w', character_mode=True),),
                      ('salam ', EciIndex(2), 'khub?', EciIndex(3)),
                      ('khoob ', EciIndex(4), '۳٫۱۴ donya?', EciIndex(5))):
            pcm, marks = bytearray(), []
            assert voice.speak(parts, pcm.extend, marks.append) and pcm
            assert marks == [p.value for p in parts if isinstance(p, EciIndex)]
    finally:
        voice.close()
        fa.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--scope-probe', type=Path)
    args = parser.parse_args()
    with DirectFirstPersian(args.native_library, args.worker,
                            args.espeak_library, args.data_parent) as frontend:
        analysis_tests(frontend)
        if args.scope_probe:
            playback_tests(frontend, args.scope_probe)
    print('Passed Pinglish direct aliases, all Latin letter names, reference positions' +
          (', native PCM.' if args.scope_probe else '.'))


if __name__ == '__main__':
    main()
