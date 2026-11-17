"""Exercise Korean pitch planning in ordinary and mixed Persian speech."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage, NATIVE_INFLECTION, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, NativeFrame
from korean_contour import KoreanContour, align_english_edges, proxy
from nvda_driver_probe import PERSIAN_PROBE, US_ENGLISH, make_language_engine
from text_frontend import SpeechPart
import continuous_driver


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, required=True, type=Path)
    args = parser.parse_args()
    assert proxy('`[.1sA.0lam] `[.1do.0nja]?') == '가가 가가?'
    assert proxy('`[.1sA.0lam], `[.1do.0nja]') == '가가, 가가.'
    assert proxy('`[.1sA.0lam]', continuation=True) == '가가,'
    assert proxy('plain English') is None
    for languages in (('en-us', 'fa'), ('fa', 'en-us')):
        rows = (NativeFrame * 240)()
        for index, row in enumerate(rows):
            row.tick = index % 120
            row.controls[1] = 120 if languages[index // 120] == 'en-us' else 160
            row.controls[2] = 50
            row.controls[9] = 400
        before = bytes(rows)
        plan = SimpleNamespace(frames=rows, counts=(120, 120))
        parts = tuple(SpeechPart(language, 'word') for language in languages)
        align_english_edges(plan, parts, lambda _parts, _edge: False)
        english_first = 0 if languages[0] == 'en-us' else 120
        assert rows[english_first + (0 if english_first == 0 else 119)].controls[1] == 120
        assert rows[119 if languages[0] == 'en-us' else 120].controls[1] == 160
        persian_first = 120 if languages[1] == 'fa' else 0
        assert all(row.controls[1] == 160 for row in rows[persian_first:persian_first + 120])
        assert all(row.controls[9] == 400 for row in rows)
        unchanged = (NativeFrame * 240).from_buffer_copy(before)
        align_english_edges(SimpleNamespace(frames=unchanged, counts=(120, 120)), parts,
                            lambda _parts, _edge: True)
        assert bytes(unchanged) == before
    original_init = FrameCapture.__init__

    def without_korean(instance, dll):
        original_init(instance, dll)
        instance.transfer_korean_pitch = None

    voice = make_language_engine(args.library, args.library, None, args.espeak_library, args.data_parent)
    cases = (
        ('سلام دنیا.',),
        ('سلام دنیا. این یک آزمایش است؟',),
        (EciIndex(1), 'سلام ', EciIndex(2), 'دنیا؟', EciIndex(3)),
        (EciIndex(4), 'NVDA ', EciIndex(5), 'سلام دنیا.', EciIndex(6)),
        (EciIndex(7), 'سلام ', EciIndex(8), 'window ', EciIndex(9), 'خوب است.', EciIndex(10)),
    )

    def speak(parts, enabled):
        pcm, marks = bytearray(), []
        def index(value):
            marks.append((value, len(pcm) // 2))
        if enabled:
            assert voice.speak(parts, pcm.extend, index)
        else:
            with patch.object(FrameCapture, '__init__', without_korean):
                assert voice.speak(parts, pcm.extend, index)
        return bytes(pcm), marks

    try:
        for rate in (1, 3, 6):
            voice.set_parameter(SAMPLE_RATE, rate)
            voice.set_parameter(NATIVE_INFLECTION, 47)
            for parts in cases:
                before, old_marks = speak(parts, False)
                after, new_marks = speak(parts, True)
                assert before and after and before != after, (rate, parts)
                assert len(before) == len(after), (rate, parts, len(before), len(after))
                assert [value for value, _ in old_marks] == [value for value, _ in new_marks]
                assert [sample for _, sample in new_marks] == sorted(sample for _, sample in new_marks)
                # Capture/replay positions are rounded to a 1 ms frame.
                assert all(abs(a[1] - b[1]) <= 55 for a, b in zip(old_marks, new_marks)), (
                    rate, parts, old_marks, new_marks)
        english = (EciLanguage(US_ENGLISH.id, '1'), 'This is an English sentence.')
        assert speak(english, False) == speak(english, True), 'English-only speech changed'
        assert voice._home[0] == PERSIAN_PROBE.id
        assert getattr(voice, '_persian_korean_contour', None) is not None
        # Verify a real mixed plan, not just the synthetic frame case: only
        # the English edge moves and its voiced endpoint meets Persian F0.
        seen = []
        real_align = continuous_driver.align_english_edges
        def observe(plan, parts, bracket):
            before = [row.controls[1] for row in plan.frames]
            real_align(plan, parts, bracket)
            offsets = [0]
            for count in plan.counts:
                offsets.append(offsets[-1] + count)
            for edge in range(1, len(parts)):
                if {parts[edge - 1].language, parts[edge].language} != {'fa', 'en-us'} or bracket(parts, edge):
                    continue
                a, b, c = offsets[edge - 1:edge + 2]
                left = next((i for i in range(b - 1, a - 1, -1)
                             if plan.frames[i].controls[2] > 10 and plan.frames[i].controls[1] > 0), None)
                right = next((i for i in range(b, c)
                              if plan.frames[i].controls[2] > 10 and plan.frames[i].controls[1] > 0), None)
                if left is None or right is None:
                    continue
                assert abs(plan.frames[left].controls[1] - plan.frames[right].controls[1]) < 0.001
                for i in range(a, c):
                    if (parts[edge - 1].language if i < b else parts[edge].language) == 'fa':
                        assert plan.frames[i].controls[1] == before[i]
                seen.append(edge)
        with patch.object(continuous_driver, 'align_english_edges', observe):
            speak((EciIndex(20), 'NVDA ', EciIndex(21), 'سلام دنیا.', EciIndex(22)), True)
            speak((EciIndex(23), 'سلام ', EciIndex(24), 'window', EciIndex(25)), True)
        assert len(seen) >= 2, 'mixed Korean switch edges were not examined'
        sentences, real_apply = [], KoreanContour.apply
        def observe_sentence(instance, rows, count, tagged, *args, **kwargs):
            sentences.append(tagged)
            return real_apply(instance, rows, count, tagged, *args, **kwargs)
        with patch.object(KoreanContour, 'apply', observe_sentence):
            speak(('سلام دنیا. این یک آزمایش است؟',), True)
        assert len(sentences) == 2 and sentences[0].rstrip().endswith('.') and sentences[1].endswith('?'), sentences
        voice.set_parameter(NATIVE_INFLECTION, 70)
        speak(('سلام دنیا.',), True)
        donor = voice._persian_korean_contour.voice
        assert donor._dll.eciGetParam(donor._handle, NATIVE_INFLECTION) == 70
    finally:
        voice.close()
    print('Passed Korean-derived Persian pitch, English isolation, lengths and NVDA positions.')


if __name__ == '__main__':
    main()
