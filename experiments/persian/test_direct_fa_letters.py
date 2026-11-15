"""Compare direct Persian letter names with the pinned eSpeak spelling."""
from __future__ import annotations

import argparse
from pathlib import Path

from direct_fa_sequence import EciIndex, EciText, ScopedDirectPersianEngine, project
from espeak_worker import EspeakWorker
from hybrid_fa_frontend import DirectFirstPersian
from reference_letter_names import LETTERS, reference_tag
from text_frontend import UnsupportedPronunciation, native_tag_accepted


def reference_tests(frontend, reference):
    for char in LETTERS:
        analysis = frontend.analyze_character(char)
        assert analysis and not analysis.used_fallback and analysis.source_map is not None
        tag = reference_tag(reference, char)
        assert analysis.parts[0].text == tag, char
        assert analysis.source_map == (0,) * len(char.encode('utf8')) + (len(tag),)
        job = project((EciIndex(1), EciText(char, character_mode=True), EciIndex(2)),
                      frontend, allow_fallback=True)
        assert job.tagged == tag and not job.needs_native_validation
        assert [part.value for part in job.parts if isinstance(part, EciIndex)] == [1, 2]
    assert frontend._worker is None, 'letter naming unexpectedly started eSpeak'
    for char in ('A', '۱', '؟', 'ك'):
        assert frontend.analyze_character(char) is None
        try:
            project((EciText(char, character_mode=True),), frontend, allow_fallback=True)
            raise AssertionError('unsupported character was accepted')
        except UnsupportedPronunciation:
            pass
    for parts in ((EciText('سلام', character_mode=True),),
                  ('سلام ', EciText('ب', character_mode=True)),
                  (EciText('ب', character_mode=True, speed_control='1'),)):
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
    set_confirmed_persian_probe()
    previous = voice._scope(1)
    try:
        for char in LETTERS:
            tag = frontend.analyze_character(char).parts[0].text
            assert native_tag_accepted(tag, voice._voice._phonemes_for_probe(tag)), char
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
