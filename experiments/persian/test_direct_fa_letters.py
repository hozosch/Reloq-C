"""Compare direct Persian letter names with the pinned eSpeak spelling."""
from __future__ import annotations

import argparse
from pathlib import Path
import re

from direct_fa_sequence import EciIndex, EciText, ScopedDirectPersianEngine, project, project_resilient
from espeak_worker import EspeakWorker
from hybrid_fa_frontend import DirectFirstPersian
from persian_symbols import SYMBOL_TAGS, UNICODE_TAG
from reference_letter_names import LETTERS, NVDA_SPOKEN_NAMES, effective_name_tag
from text_frontend import UnsupportedPronunciation, native_sequence_accepted


def reference_tests(frontend, reference):
    for char in LETTERS:
        analysis = frontend.analyze_character(char)
        assert analysis and not analysis.used_fallback and analysis.source_map is not None
        tag = effective_name_tag(reference, char)
        if char in NVDA_SPOKEN_NAMES:
            expanded = frontend.analyze_direct(NVDA_SPOKEN_NAMES[char])
            if expanded is not None and len(expanded.parts) == 1:
                tag = expanded.parts[0].text
        tag = re.sub(r"'([Aaeiou]):'", r'\1', tag)
        assert analysis.parts[0].text == tag, char
        assert analysis.source_map == (0,) * len(char.encode('utf8')) + (len(tag),)
        job = project((EciIndex(1), EciText(char, character_mode=True), EciIndex(2)),
                      frontend, allow_fallback=True)
        assert job.tagged == tag and not job.needs_native_validation
        assert [part.value for part in job.parts if isinstance(part, EciIndex)] == [1, 2]
        if char in 'ًٌٍَُِّْٰٔ':
            # A combining mark sent alone must also be named if the caller
            # did not explicitly select character mode.
            assert project_resilient((char,), frontend).tagged == tag, char
    assert frontend._worker is None, 'letter naming unexpectedly started eSpeak'
    for char, name in NVDA_SPOKEN_NAMES.items():
        # NVDA may replace a single character with its spoken label before it
        # reaches the driver; both inputs must produce the same Persian tags.
        literal = project((EciText(char, character_mode=True),), frontend, allow_fallback=True)
        expanded = project((name,), frontend, allow_fallback=True)
        assert literal.tagged == expanded.tagged, (char, name)
        assert not literal.needs_native_validation and not expanded.needs_native_validation
    assert frontend._worker is None, 'character labels unexpectedly started eSpeak'
    word = (EciText('سل', character_mode=True), EciIndex(3),
            EciText('ام', character_mode=True), EciIndex(4))
    job = project(word, frontend, allow_fallback=True)
    assert job.tagged == ' '.join(frontend.analyze_character(c).parts[0].text for c in 'سلام')
    assert [part.value for part in job.parts if isinstance(part, EciIndex)] == [3, 4]
    for char in (*SYMBOL_TAGS, 'A', 'ك', '🙂'):
        name = frontend.analyze_character(char)
        assert name is not None and name.parts[0].text
        if char in SYMBOL_TAGS:
            assert name.parts[0].text == re.sub(r"'([Aaeiou]):'", r'\1', SYMBOL_TAGS[char])
        assert name.source_map == (0,) * len(char.encode('utf8')) + (len(name.parts[0].text),)
        job = project((EciIndex(5), EciText(char, character_mode=True), EciIndex(6)),
                      frontend, allow_fallback=True)
        assert job.tagged == name.parts[0].text
        assert [part.value for part in job.parts if isinstance(part, EciIndex)] == [5, 6]
    assert frontend.analyze_character('ك').parts == frontend.analyze_character('ک').parts
    assert frontend.analyze_character('🙂').parts[0].text == SYMBOL_TAGS['🙂']
    mixed = project(('سلام ', EciIndex(7), EciText('؟', character_mode=True),
                     EciIndex(8), ' دنیا'), frontend, allow_fallback=True)
    assert mixed.tagged == (frontend.analyze_direct('سلام ').parts[0].text +
                            frontend.analyze_character('؟').parts[0].text +
                            frontend.analyze_direct(' دنیا').parts[0].text)
    assert [part.value for part in mixed.parts if isinstance(part, EciIndex)] == [7, 8]
    for parts in ((EciText('ب', character_mode=True, speed_control='1'),),):
        try:
            project(parts, frontend, allow_fallback=True)
            raise AssertionError('unsupported spelling job was accepted')
        except UnsupportedPronunciation:
            pass


def playback_tests(frontend, probe, reference):
    from listening import render
    from test_selected_probe import engine, set_confirmed_persian_probe
    from _reloq_eci_direct import GENDER, SAMPLE_RATE

    voice = ScopedDirectPersianEngine(probe, frontend, allow_fallback=True)
    fa = engine(reference, 0x90000)
    set_confirmed_persian_probe(d=2, z=2)
    previous = voice._scope(1)
    try:
        for char in LETTERS:
            tag = frontend.analyze_character(char).parts[0].text
            assert native_sequence_accepted(tag, voice._voice._phonemes_for_probe(tag)), char
    finally:
        voice._scope(previous)
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for target in (voice, fa):
                    target.set_parameter(SAMPLE_RATE, rate)
                    target.set_parameter(GENDER, gender)
                for char in ('ا', 'ب', 'ر', 'گ', 'غ', 'ق', 'ث'):
                    tag = frontend.analyze_character(char).parts[0].text
                    expected = render(fa, tag)
                    pcm, marks = bytearray(), []
                    parts = (EciIndex(1), EciText(char, character_mode=True), EciIndex(2))
                    assert voice.speak(parts, pcm.extend, lambda i: marks.append((i, len(pcm) // 2)))
                    assert bytes(pcm) == expected and marks == [(1, 0), (2, len(pcm) // 2)], char
                for char in ('؟', '۱', ' ', '🙂'):
                    pcm, marks = bytearray(), []
                    assert voice.speak((EciIndex(3), EciText(char, character_mode=True), EciIndex(4)),
                                       pcm.extend, marks.append) and pcm
                    assert marks == [3, 4]
        unmarked, marked, marks = bytearray(), bytearray(), []
        assert voice.speak(('سلام ', EciText('؟', character_mode=True), ' دنیا'),
                           unmarked.extend, lambda _: None)
        assert voice.speak(('سلام ', EciIndex(8), EciText('؟', character_mode=True),
                            EciIndex(9), ' دنیا'), marked.extend, marks.append)
        assert marked == unmarked and marks == [8, 9]
        assert frontend._worker is None
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
    parser.add_argument('--reference', type=Path)
    args = parser.parse_args()
    if bool(args.scope_probe) != bool(args.reference):
        parser.error('provide both acoustic libraries or neither')
    with DirectFirstPersian(args.native_library, args.worker,
                            args.espeak_library, args.data_parent) as frontend:
        with EspeakWorker(args.worker, args.espeak_library, args.data_parent) as reference:
            reference_tests(frontend, reference)
        if args.scope_probe:
            playback_tests(frontend, args.scope_probe, args.reference)
    print(f'Passed {len(LETTERS)} direct Persian letter names' +
          (', native sound and indexes.' if args.scope_probe else '.'))


if __name__ == '__main__':
    main()
