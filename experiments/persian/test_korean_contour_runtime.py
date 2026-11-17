"""Exercise Korean pitch planning in ordinary and mixed Persian speech."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from types import SimpleNamespace
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage, EciPitch, EciPitchHz, EciProsody, EciText, GENDER, NATIVE_INFLECTION, NATIVE_PITCH, PITCH_HZ, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture, NativeFrame
from korean_contour import KoreanContour, align_english_edges, korean_inflection, korean_pitch, proxy
import korean_contour
from nvda_driver_probe import PERSIAN_PROBE, US_ENGLISH, make_language_engine
from text_frontend import SpeechPart
from compare_korean_long import SENTENCES
from direct_fa_sequence import project_resilient
import continuous_driver


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, required=True, type=Path)
    args = parser.parse_args()
    assert proxy('`[.1sA.0lam] `[.1do.0nja]?') == '가가가가?'
    assert proxy('`[.1sA.0lam], `[.1do.0nja]') == '가가, 가가.'
    assert proxy('`[.1sA.0lam]') == '가가.'
    assert proxy('`[.1sA.0lam]', continuation=True) == '가가,'
    assert proxy('`[.1sA.0lam],') == '가가,'
    assert proxy('`[.1sA.0lam]:') == proxy('`[.1sA.0lam].')
    assert proxy('`[salam]') == '가.'
    opening = '`[.0pa.0ra.1an.0tes] `[.1bAs]'
    closing = '`[.0pa.0ra.1an.0tes] `[.1bAs.0te]'
    assert proxy(opening) == '가가가가 가.'
    assert proxy(closing) == '가가가가 가가.'
    assert proxy(opening, continuation=True) == '가가가가가,'
    assert proxy(opening + '?') == '가가가가가?'
    assert proxy('`[.0a.1b.0c]') == '가 가가.'
    assert proxy('`[.0a.2b.0c]') == '가 가가.'
    for tagged, count, punctuation in (
            ('`[.1sa.0lam] `[.1do.0nja].', 4, '.'),
            ('`[.0be] `[.1mAr.0dom], `[.1sA.0lam]?', 5, ',?'),
            ('`[.0a.0b.1c.0d.0e.0f] `[.1g]', 7, '.'),
            ('`[foo] `[.1bar]!', 2, '!')):
        result = proxy(tagged)
        assert result.count('가') == count and ''.join(c for c in result if c in ',.!?') == punctuation
    mapped = [korean_inflection(value) for value in range(101)]
    assert mapped[0] == 0 and mapped[30] == 43 and mapped[100] == 100
    assert all(a <= b for a, b in zip(mapped, mapped[1:]))
    pitches = [korean_pitch(value) for value in range(101)]
    assert pitches[0] == 0 and pitches[65] == 68 and pitches[100] == 100
    assert all(a <= b for a, b in zip(pitches, pitches[1:]))
    for text, boundary in (('سلام:دنیا', True), ('سلام: دنیا', True),
                           ('12:30', False), ('۱۲ : ۳۰', False),
                           ('https://example.org', False), ('C:\\\\file', False)):
        assert continuous_driver._colon_ends_sentence(text, text.index(':')) == boundary, text
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
        # An individual unsupported character uses its real English name,
        # with the same language boundaries and indexes as an explicit switch.
        rare = (EciIndex(50), 'سلام ', EciIndex(51), EciText('Ж', character_mode=True),
                EciIndex(52), ' دنیا.', EciIndex(53))
        named = (EciIndex(50), 'سلام ', EciIndex(51), EciLanguage(US_ENGLISH.id, '1'),
                 'cyrillic capital letter zhe', EciIndex(52),
                 EciLanguage(PERSIAN_PROBE.id, '1'), ' دنیا.', EciIndex(53))
        assert speak(rare, True) == speak(named, True), 'character fallback changed language routing or indexes'
        assert speak((EciText('Ж', character_mode=True),), True)[0] == speak(
            (EciLanguage(US_ENGLISH.id, '1'), 'cyrillic capital letter zhe'), True)[0]
        multi = (EciIndex(54), EciText('Ж؟', character_mode=True), EciIndex(55))
        multi_named = (EciIndex(54), EciLanguage(US_ENGLISH.id, '1'),
                       EciText('cyrillic capital letter zhe'), EciLanguage(PERSIAN_PROBE.id, '1'),
                       EciText('؟', character_mode=True), EciIndex(55))
        assert speak(multi, True) == speak(multi_named, True), 'mixed character spelling changed'
        assert voice._home[0] == PERSIAN_PROBE.id
        assert getattr(voice, '_persian_korean_contour', None) is not None
        # Match the previous candidate at the user's actual slider settings,
        # not merely at the donor's internal Korean inflection value.
        def old_inflection(value):
            return ((value * 37 + 15) // 30 if value <= 30 else
                    37 + ((value - 30) * 63 + 35) // 70)
        for sample_rate in (1, 3, 6):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            for gender in (0, 1):
                voice.set_parameter(GENDER, gender)
                voice.set_parameter(NATIVE_PITCH, 68)
                voice.set_parameter(NATIVE_INFLECTION, 37)
                with (patch.object(korean_contour, 'korean_pitch', lambda value: value),
                      patch.object(korean_contour, 'korean_inflection', old_inflection)):
                    reference, _ = speak(('سلام دنیا. این یک آزمایش است؟',), True)
                voice.set_parameter(NATIVE_PITCH, 65)
                voice.set_parameter(NATIVE_INFLECTION, 30)
                current, _ = speak(('سلام دنیا. این یک آزمایش است؟',), True)
                assert current == reference, ('65/30 differs from previous 68/37', sample_rate, gender)
        voice.set_parameter(SAMPLE_RATE, 3)
        voice.set_parameter(GENDER, 0)
        plain, _ = speak(('سلام دنیا.',), True)
        for prefix in (EciPitch(65), EciText('`vb65 ', tag_control='`vb65'),
                       EciText('`vf30 ', tag_control='`vf30')):
            assert speak((prefix, 'سلام دنیا.'), True)[0] == plain, prefix
        pitch_examples = []
        for value in (50, 65, 80):
            voice.set_parameter(NATIVE_PITCH, value)
            pitch_examples.append(speak(('سلام دنیا.',), True)[0])
        assert len(set(pitch_examples)) == len(pitch_examples), 'pitch slider stopped affecting Persian'
        voice.set_parameter(NATIVE_PITCH, 68)
        voice.set_parameter(NATIVE_INFLECTION, 37)
        voice.set_parameter(PITCH_HZ, 180)
        with (patch.object(korean_contour, 'korean_pitch', lambda value: value),
              patch.object(korean_contour, 'korean_inflection', old_inflection)):
            old_hertz, _ = speak(('سلام دنیا.',), True)
        voice.set_parameter(NATIVE_PITCH, 65)
        voice.set_parameter(NATIVE_INFLECTION, 30)
        voice.set_parameter(PITCH_HZ, 180)
        assert speak(('سلام دنیا.',), True)[0] == old_hertz, 'exact Hertz pitch changed'
        voice.set_parameter(PITCH_HZ, 0)
        voice.set_parameter(NATIVE_PITCH, 65)
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
        # A colon in prose must restart the Korean planner like a full stop,
        # including without spaces and through real Persian/English routing.
        # Compare real output, not only the generated donor text.
        for sample_rate in (1, 3, 6):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            for colon, full_stop in (
                    ('سلام دنیا: این یک آزمایش است؟', 'سلام دنیا. این یک آزمایش است؟'),
                    ('سلام:دنیا', 'سلام.دنیا'),
                    ('سلام دنیا: NVDA سلام.', 'سلام دنیا. NVDA سلام.')):
                recordings, donors = [], []
                for text in (colon, full_stop):
                    sentences.clear()
                    with patch.object(KoreanContour, 'apply', observe_sentence):
                        recordings.append(speak((text,), True)[0])
                    donors.append(tuple(proxy(tagged) for tagged in sentences))
                assert len(donors[0]) == 2 and donors[0] == donors[1], (colon, donors)
                assert recordings[0] == recordings[1], (sample_rate, colon)
        voice.set_parameter(SAMPLE_RATE, 3)
        for text in ('ساعت 12:30 است.', 'ساعت ۱۲:۳۰ است.'):
            sentences.clear()
            with patch.object(KoreanContour, 'apply', observe_sentence):
                speak((text,), True)
            assert len(sentences) == 1, ('time was split into sentences', text, sentences)
        colon_parts = (EciIndex(41), 'سلام دنیا:', EciIndex(42), ' این ',
                       EciIndex(43), 'یک آزمایش است؟', EciIndex(44))
        sentences.clear()
        with patch.object(KoreanContour, 'apply', observe_sentence):
            colon_indexed, marks = speak(colon_parts, True)
        assert len(sentences) == 2 and [mark for mark, _ in marks] == [41, 42, 43, 44]
        assert colon_indexed == speak(('سلام دنیا: این یک آزمایش است؟',), True)[0]
        # The original accepted listening sample inserted a mark only at
        # each sentence boundary. With a mark before every word, the native
        # callback can report some sentence starts inside the previous word.
        # Compare real target frame spans, including densely indexed NVDA
        # input and a U-to-A glide, against independently captured sentence
        # marks instead of merely checking which tags reached the planner.
        voice.set_parameter(NATIVE_INFLECTION, 30)
        engine = voice._activate(voice._home)
        def sentence_reference(texts):
            parts = tuple(part for i, text in enumerate(texts, 1)
                          for part in (EciIndex(i), text + ' '))
            job = project_resilient(parts, engine._frontend, pinglish=engine._pinglish)
            target = continuous_driver._PlanningVoice(voice, voice._home, engine, source=job.source)
            captured = FrameCapture(target._dll).capture(target, job.parts, maximum=30000, audio=False)
            assert captured and len(captured.indexes) == len(texts)
            return [round(sample * 1000 / SAMPLE_RATES[sample_rate]) for _, sample in captured.indexes]
        def sentence_spans(parts):
            found = []
            def observe_span(instance, rows, count, tagged, *args, **kwargs):
                result = real_apply(instance, rows, count, tagged, *args, **kwargs)
                if result:
                    found.append((rows[0].tick, rows[count - 1].tick, tagged))
                return result
            with patch.object(KoreanContour, 'apply', observe_span):
                pcm, positions = speak(parts, True)
            assert pcm
            return found, positions, pcm
        long_text = tuple(text for text, _ in SENTENCES)
        for sample_rate in (1, 3, 6):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            for texts in ((long_text, ('او آمد.', *long_text)) if sample_rate == 3 else (long_text,)):
                expected = sentence_reference(texts)
                full = ' '.join(texts)
                spans, _, plain_pcm = sentence_spans((full,))
                assert [row[0] for row in spans] == expected, (sample_rate, expected, spans)
                dense = []
                for index, word in enumerate(full.split(), 100):
                    dense.extend((EciIndex(index), word + ' '))
                spans, marks, dense_pcm = sentence_spans(tuple(dense))
                assert [row[0] for row in spans] == expected, (sample_rate, expected, spans)
                assert dense_pcm == plain_pcm, 'source indexes changed the Korean sentence melody'
                assert [number for number, _ in marks] == list(range(100, 100 + len(dense) // 2))
                assert [sample for _, sample in marks] == sorted(sample for _, sample in marks)
                if sample_rate == 3 and texts == long_text:
                    pitched = []
                    for index, word in enumerate(full.split(), 100):
                        if index in (105, 112):
                            pitched.append(EciPitch(80 if index == 105 else 65))
                        pitched.extend((EciIndex(index), word + ' '))
                    spans, marks, _ = sentence_spans(tuple(pitched))
                    assert [row[0] for row in spans] == expected, (expected, spans)
                    assert [number for number, _ in marks] == list(range(100, 100 + len(dense) // 2))
        voice.set_parameter(SAMPLE_RATE, 6)
        voice.set_parameter(NATIVE_INFLECTION, 47)
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
        voice.set_parameter(NATIVE_INFLECTION, 30)
        # Verify that the actual Korean core's first and last voiced F0 are
        # transferred directly: no phrase-specific endpoint or reference
        # curve is substituted for the Korean planner.
        endpoints = []
        def observe_direct(instance, rows, count, tagged, *args, **kwargs):
            result = real_apply(instance, rows, count, tagged, *args, **kwargs)
            values = [rows[i].controls[1] for i in range(count)
                      if rows[i].controls[2] > 10 and rows[i].controls[1] > 0]
            if result and values:
                endpoints.append((values[0], values[-1]))
            return result
        with patch.object(KoreanContour, 'apply', observe_direct):
            speak(('سلام دنیا.',), True)
        native = FrameCapture(donor._dll).capture(donor, ('가가 가가.',), audio=False)
        values = [row.controls[1] for row in native.frames()
                  if row.controls[2] > 10 and row.controls[1] > 0]
        assert len(endpoints) == 1 and values
        assert abs(endpoints[0][0] - values[0]) < 0.01 and abs(endpoints[0][1] - values[-1]) < 0.01
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
                      (EciText('سلام '), EciPitch(80), EciText('دنیا.'), EciPitch(65)),
                      (EciText('سلام '), EciPitchHz(220), EciText('دنیا.')),
                      (EciText('سلام '), EciProsody(6, 35), EciText('دنیا.'))):
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
