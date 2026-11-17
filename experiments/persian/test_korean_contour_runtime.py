"""Exercise Korean pitch planning in ordinary and mixed Persian speech."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage, EciPitch, EciPitchHz, EciText, NATIVE_INFLECTION, PITCH_HZ, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, NativeFrame
import korean_contour
from korean_contour import KoreanContour, align_english_edges, korean_inflection, proxy
from nvda_driver_probe import PERSIAN_PROBE, US_ENGLISH, make_language_engine
from text_frontend import SpeechPart
import continuous_driver


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, required=True, type=Path)
    args = parser.parse_args()
    assert proxy('`[.1sA.0lam] `[.1do.0nja]?') == '가가 가가?'
    assert proxy('`[.1sA.0lam], `[.1do.0nja]') == '가가, 가가가.'
    assert proxy('`[.1sA.0lam]') == '가가가.'
    assert proxy('`[.1sA.0lam]', continuation=True) == '가가,'
    assert proxy('`[.1sA.0lam],') == '가가,'
    assert proxy('`[salam]') == '가.'
    mapped = [korean_inflection(value) for value in range(101)]
    assert mapped[0] == 0 and mapped[30] == 37 and mapped[100] == 100
    assert all(a <= b for a, b in zip(mapped, mapped[1:]))
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
        boundary_pitch = rows[119 if languages[0] == 'en-us' else 120].controls[1]
        assert 120 < boundary_pitch <= 120 * 2 ** (2 / 12) + 0.001
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
        ('سلام',),
        ('کتاب',),
        ('یک',),
        (EciText('سلام', character_mode=True),),
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
                english = left if parts[edge - 1].language == 'en-us' else right
                persian = right if english == left else left
                fitted = plan.frames[english].controls[1]
                assert abs(fitted - before[english]) <= before[english] * (2 ** (2 / 12) - 1) + 0.001
                assert abs(fitted - plan.frames[persian].controls[1]) <= abs(
                    before[english] - plan.frames[persian].controls[1]) + 0.001
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
        # Sentence punctuation and inline pitch can force the separated
        # multilingual route. Its Persian engine must still use Korean pitch.
        fallback_cases = (
            ('Hello. سلام دنیا',),
            ('Hello، سلام دنیا',),
            (EciText('Hello '), EciPitch(80), EciText('سلام'), EciPitch(65)),
            ('Hello ', EciLanguage(0x40000, '1'), 'Hallo ',
             EciLanguage(PERSIAN_PROBE.id, '1'), 'سلام دنیا'),
        )
        for parts in fallback_cases:
            sentences.clear()
            with patch.object(KoreanContour, 'apply', observe_sentence):
                audio, _ = speak(parts, True)
            assert audio and sentences, ('separated Persian lost Korean contour', parts)
        # A Persian span followed by English must retain a continuation
        # contour rather than acquiring a Korean phrase-final fall.
        endings = []
        def observe_end(instance, rows, count, tagged, *args, **kwargs):
            result = real_apply(instance, rows, count, tagged, *args, **kwargs)
            if result:
                pitches = [rows[i].controls[1] for i in range(count)
                           if rows[i].controls[2] > 10 and rows[i].controls[1] > 0]
                endings.append((bool(kwargs.get('continuation')), pitches[-1]))
            return result
        with patch.object(KoreanContour, 'apply', observe_end):
            speak(('سلام',), True)
            speak(('سلام NVDA',), True)
        normal = next(pitch for continuation, pitch in endings if not continuation)
        continuing = next(pitch for continuation, pitch in endings if continuation)
        assert continuing > normal * 1.1, endings
        donor = voice._persian_korean_contour.voice
        examples = []
        for inflection in (30, 37, 47, 70):
            voice.set_parameter(NATIVE_INFLECTION, inflection)
            examples.append(speak(('سلام دنیا.',), True)[0])
            assert donor._dll.eciGetParam(donor._handle, NATIVE_INFLECTION) == korean_inflection(inflection)
        assert len(set(examples)) == len(examples), 'the Korean donor ignored an inflection setting'
        # Check the actual Korean donor and replay, rather than only checking
        # that the planner was called. The old two-syllable synthetic ending
        # dipped sharply; the contextual donor must change pitch, not length,
        # while questions and an English continuation retain their old PCM.
        def old_syllables(matches, _tagged, *, continuation=False):
            return tuple(max(1, len(korean_contour._SYLLABLE.findall(match.group()))) for match in matches)
        voice.set_parameter(NATIVE_INFLECTION, 30)
        for parts in (('سلام',), ('سلام دنیا.',)):
            with patch.object(korean_contour, '_donor_syllables', old_syllables):
                old, old_marks = speak(parts, True)
            new, new_marks = speak(parts, True)
            assert old != new and len(old) == len(new) and old_marks == new_marks, parts
        for parts in (('سلام؟',), ('سلام NVDA',)):
            with patch.object(korean_contour, '_donor_syllables', old_syllables):
                old, old_marks = speak(parts, True)
            assert speak(parts, True) == (old, old_marks), parts
        for inflection in (0, 30, 70, 100):
            voice.set_parameter(NATIVE_INFLECTION, inflection)
            final_pitches = []
            def observe_final(instance, rows, count, tagged, *args, **kwargs):
                result = real_apply(instance, rows, count, tagged, *args, **kwargs)
                if result:
                    final_pitches.extend(rows[i].controls[1] for i in range(count)
                                         if rows[i].controls[2] > 10 and rows[i].controls[1] > 0)
                return result
            with patch.object(KoreanContour, 'apply', observe_final):
                speak(('سلام',), True)
            assert final_pitches and donor._dll.eciGetParam(donor._handle, NATIVE_INFLECTION) == korean_inflection(inflection)
        voice.set_parameter(NATIVE_INFLECTION, 47)
        # An NVDA pitch command inside Persian used to bypass the Korean
        # planner for the entire utterance. Check its actual replayed F0 and
        # non-pitch controls against an otherwise identical neutral request.
        replayed = []
        native_replay = continuous_driver.FrameReplay.replay
        def observe_replay(instance, source_voice, frames, marks, *args, **kwargs):
            replayed.append(tuple((row.tick, tuple(row.controls)) for row in frames))
            return native_replay(instance, source_voice, frames, marks, *args, **kwargs)
        plain_parts = (EciText('سلام '), EciText('دنیا.'))
        raised_parts = (EciText('سلام '), EciPitch(80), EciText('دنیا.'), EciPitch(65))
        with patch.object(continuous_driver.FrameReplay, 'replay', observe_replay):
            normal_pcm, _ = speak(plain_parts, True)
            raised_pcm, _ = speak(raised_parts, True)
        assert len(replayed) == 2 and len(normal_pcm) == len(raised_pcm) and normal_pcm != raised_pcm
        normal_rows, raised_rows = replayed
        assert len(normal_rows) == len(raised_rows)
        assert all(a[0] == b[0] and all(a[1][i] == b[1][i] for i in range(62) if i != 1)
                   for a, b in zip(normal_rows, raised_rows))
        assert abs(normal_rows[150][1][1] - raised_rows[150][1][1]) < 0.01
        assert raised_rows[450][1][1] > normal_rows[450][1][1] * 1.3
        # Older ECI libraries normalize the transferred pitch to the target
        # median. They must not count the inline pitch adjustment twice.
        replayed.clear()
        with patch.object(voice._persian_korean_contour.api, 'transfer_korean_aligned', None), \
             patch.object(continuous_driver.FrameReplay, 'replay', observe_replay):
            speak(plain_parts, True)
            speak(raised_parts, True)
        assert len(replayed) == 2
        assert abs(replayed[0][150][1][1] - replayed[1][150][1][1]) < 0.01
        assert replayed[1][450][1][1] > replayed[0][450][1][1] * 1.3
        for parts in ((EciIndex(90), EciText('سلام '), EciIndex(91), EciPitch(80),
                       EciText('دنیا '), EciPitch(65), EciIndex(92), EciText('خوب.')),
                      (EciText('سلام '), EciPitch(80), EciText('دنیا.'), EciPitch(65))):
            calls = []
            def observe_inline(instance, rows, count, tagged, *args, **kwargs):
                result = real_apply(instance, rows, count, tagged, *args, **kwargs)
                if result:
                    calls.append(tagged)
                return result
            with patch.object(KoreanContour, 'apply', observe_inline):
                actual, marks = speak(parts, True)
            original, original_marks = speak(parts, False)
            assert calls and len(actual) == len(original) and actual != original, parts
            assert [value for value, _ in marks] == [value for value, _ in original_marks]
            assert all(abs(a[1] - b[1]) <= 55 for a, b in zip(marks, original_marks))
        # Request-local pitch controls and the persistent Hertz mode must
        # reach the Korean donor; a following ordinary request must reset.
        neutral, _ = speak(('سلام',), True)
        high, _ = speak((EciPitch(85), 'سلام'), True)
        assert neutral != high
        assert speak((EciPitch(85), 'سلام', EciPitch(65)), True)[0] == high
        assert speak(('سلام',), True)[0] == neutral
        voice.set_parameter(PITCH_HZ, 180)
        hertz, _ = speak(('سلام',), True)
        assert hertz != neutral and donor._pitch_hz == 180
        inline, _ = speak((EciPitchHz(240), 'سلام'), True)
        assert inline != hertz and speak(('سلام',), True)[0] == hertz
        voice.set_parameter(PITCH_HZ, 0)
    finally:
        voice.close()
    print('Passed Korean-derived Persian pitch, English isolation, lengths and NVDA positions.')


if __name__ == '__main__':
    main()
