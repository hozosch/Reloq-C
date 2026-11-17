"""Verify the Persian US-pilot experiment, including weak and final vowels."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, EciPitch, NATIVE_INFLECTION, NATIVE_PITCH, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from continuous_driver import _PlanningVoice, _sentence_colons_as_periods
from continuous_plan import CapturedRows, fit_us_word_pitch, indexed_us_proxy
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH
import continuous_driver
import continuous_plan


def index_words(tagged, marks):
    words = tuple(re.finditer(r'`\[[^]]+\]', tagged))
    assert len(words) == len(marks) + 1
    parts, cursor = [], 0
    for word, mark in zip(words[1:], marks):
        parts.extend((tagged[cursor:word.start()], EciIndex(mark)))
        cursor = word.start()
    return (*parts, tagged[cursor:])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    voice = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    voice.set_parameter(SAMPLE_RATE, 3)
    voice.set_parameter(NATIVE_PITCH, 65)
    voice.set_parameter(NATIVE_INFLECTION, 30)
    fa_key = voice._key(PERSIAN_PROBE.id, '1')
    en_key = voice._key(US_ENGLISH.id, '1')
    fa_engine = voice._activate(fa_key)
    fa = _PlanningVoice(voice, fa_key, fa_engine)
    en = _PlanningVoice(voice, en_key, voice._activate(en_key))
    en.raw.set_parameter(PROCESS_TAGS, 1)
    api = FrameCapture(fa._dll)

    def render(parts):
        pcm, indexes = bytearray(), []
        assert voice.speak(parts, pcm.extend, lambda mark: indexes.append((mark, len(pcm))))
        assert pcm
        return bytes(pcm), indexes

    try:
        # In every tested word, only F0 changes; the US end value reaches
        # the very last voiced frame, including an open final vowel.
        for text in ('دو.', 'دو؟', 'خانه.', 'سلام دنیا.', 'کتاب و خانه.', 'کتاب را.'):
            tagged = project((text,), fa_engine._frontend, allow_fallback=False).tagged
            count = len(re.findall(r'`\[[^]]+\]', tagged))
            marks = tuple(range(1, count))
            target = CapturedRows.from_capture(api.capture(fa, index_words(tagged, marks), audio=False))
            donor = CapturedRows.from_capture(api.capture(en, indexed_us_proxy(tagged, en, marks), audio=False))
            before = [tuple(row.controls) for row in target.frames]
            assert fit_us_word_pitch(target, (0, len(target.pcm) // 2), donor,
                                     (0, len(donor.pcm) // 2), marks, marks, 16000, api), text
            assert any(old[1] != row.controls[1] for old, row in zip(before, target.frames)), text
            assert all(all(old[i] == row.controls[i] for i in range(62) if i != 1)
                       for old, row in zip(before, target.frames)), text
            target_end = next(row.controls[1] for row in reversed(target.frames)
                              if row.controls[2] > 0 and row.controls[1] > 0)
            donor_end = next(row.controls[1] for row in reversed(donor.frames)
                             if row.controls[2] > 0 and row.controls[1] > 0)
            assert abs(target_end - donor_end) < 3, (text, target_end, donor_end)

        weak = project(('کتاب را.',), fa_engine._frontend, allow_fallback=False).tagged
        assert '`00 `[.0ra]' in weak
        assert '`00' in en._phonemes_for_probe(''.join(
            part for part in indexed_us_proxy(weak, en, (1,)) if isinstance(part, str)))
        assert project(('را',), fa_engine._frontend, allow_fallback=False).tagged == '`[.1ra]'
        assert project(('اسلش',), fa_engine._frontend, allow_fallback=False).tagged == '`[.0es.1lAS]'
        assert fa_engine._frontend.analyze_character('/').parts[0].text == '`[.0es.1lAS]'
        assert fa_engine._frontend.analyze_character('\\').parts[0].text.endswith('`[.0es.1lAS]')
        pinglish = voice._activate(voice._key(PINGLISH_PROBE.id, '1'))
        assert project(('eslash',), pinglish._frontend, pinglish=True).tagged == '`[.0es.1lAS]'
        assert _sentence_colons_as_periods('`[.1do]: `[.1se]', 'دو: سه') == '`[.1do]. `[.1se]'
        assert _sentence_colons_as_periods('`[.1do]:`[.1se]', '2:3') == '`[.1do]:`[.1se]'
        assert render(('سلام (دنیا) خوب',))[0] == render(('سلام، دنیا، خوب',))[0]
        assert render(('سلام دنیا: این یک آزمایش است؟',))[0] == render(
            ('سلام دنیا. این یک آزمایش است؟',))[0]

        # Exercise the same driver paths used by NVDA for isolated, mixed,
        # and inline-pitch speech; failure to fit must not silently pass.
        calls = []
        def observe(*args, **kwargs):
            result = fit_us_word_pitch(*args, **kwargs)
            calls.append(result)
            return result
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=observe), \
             patch.object(continuous_plan, 'fit_us_word_pitch', side_effect=observe):
            for parts in ((EciIndex(1), 'دو.', EciIndex(2)),
                          (EciIndex(1), 'کتاب و خانه.', EciIndex(2)),
                          (EciIndex(1), 'NVDA کتاب', EciIndex(2)),
                          (EciIndex(1), 'کتاب NVDA خانه.', EciIndex(2)),
                          (EciIndex(1), 'کتاب ', EciPitch(75), 'خانه.', EciIndex(2))):
                calls.clear()
                pcm, indexes = render(parts)
                assert calls and all(calls), (parts, calls)
                assert indexes[0] == (1, 0) and indexes[-1] == (2, len(pcm)), (parts, indexes)
    finally:
        voice.close()
    print('Passed US/Persian final-vowel pitch, weak words, word names and NVDA routing.')


if __name__ == '__main__':
    main()
