"""Check Persian colon boundaries and US proxy timing for number 96."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture
from continuous_driver import _PlanningVoice, _colon_ends_sentence, sentence_jobs
from continuous_plan import CapturedRows, fit_us_word_pitch, indexed_us_proxy, persian_proxy_for_english, word_boundary
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH
from test_persian_syllable_pitch import ObservedApi
import continuous_driver
import continuous_plan


def vowel_start(item, syllable):
    """Locate the longest voiced run; do not confuse a short preceding tail."""
    left, right = (value / 10000 for value in item.syllables[syllable])
    runs, run = [], []
    for row in item.frames:
        if left <= row.tick < right and row.controls[2] > 5:
            run.append(row.tick)
        elif run:
            runs.append(run)
            run = []
    if run:
        runs.append(run)
    assert runs
    return max(runs, key=len)[0] - left


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    for text, expected in (
            ('12:30', False), ('۱۲:۳۰', False), ('12: 30', True),
            ('https://nvaccess.org', False), ('C:\\Windows', False),
            ('file:/C:/windows', False), ('custom:/path', False),
            ('NVDA: / دنیا', True), ('NVDA:/ دنیا', True),
            ('سلام:/ دنیا', True), ('سلام:', True)):
        assert _colon_ends_sentence(text, text.index(':')) is expected, text
    items = ('NVDA', EciIndex(7), ': ', EciIndex(8), '/ دنیا')
    jobs = sentence_jobs(items, (PERSIAN_PROBE.id, '1'))
    assert len(jobs) == 2
    assert [part.value for job in jobs for part in job if isinstance(part, EciIndex)] == [7, 8]

    voice = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    voice.set_parameter(NATIVE_PITCH, 65)
    voice.set_parameter(NATIVE_INFLECTION, 30)
    fk, ek = voice._key(PERSIAN_PROBE.id, '1'), voice._key(US_ENGLISH.id, '1')
    engine = voice._activate(fk)
    fa, en = _PlanningVoice(voice, fk, engine), _PlanningVoice(voice, ek, voice._activate(ek))
    en.raw.set_parameter(PROCESS_TAGS, 1)
    api = FrameCapture(fa._dll)
    try:
        for sample_rate in (0, 3, 6):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            for speed in (45, 65, 90):
                voice.set_parameter(NATIVE_SPEED, speed)
                tagged = project(('96',), engine._frontend, allow_fallback=False).tagged
                proxy = indexed_us_proxy(tagged, en, ())
                displayed = en._phonemes_for_probe(''.join(p for p in proxy if isinstance(p, str)))
                words = re.findall(r'`\[([^]]+)\]', displayed)
                assert len(words) == 1 and re.search(r'\.1S[^.]*S$', words[0]), (proxy, displayed)
                target = CapturedRows.from_capture(api.capture(fa, (tagged,), audio=False))
                donor = CapturedRows.from_capture(api.capture(en, proxy, audio=False))
                assert len(target.syllables) == len(donor.syllables) == 4
                assert abs(vowel_start(target, -1) - vowel_start(donor, -1)) <= 15, (sample_rate, speed)

        # The US parser moves one /s/ to the next syllable in 66 and changes
        # the first unstressed vowel of parantez. Neither is a reason to
        # replace the whole silent pilot word with unrelated /m/ syllables.
        # Its stop/fricative intervals must remain aligned with the Persian
        # vowels, and the observed syllable anchors must actually be used.
        pinglish_key = voice._key(PINGLISH_PROBE.id, '1')
        pinglish_engine = voice._activate(pinglish_key)
        pinglish_voice = _PlanningVoice(voice, pinglish_key, pinglish_engine)
        for sample_rate in (0, 3, 6):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            for speed in (45, 82, 90):
                voice.set_parameter(NATIVE_SPEED, speed)
                for text, carrier, frontend, latin, expected in (
                        ('66', fa, engine._frontend, False, '`[.0SAs.0to.1SES]'),
                        ('پرانتز باز', fa, engine._frontend, False, '`[.0pA.0ran.1tEz]'),
                        ('parantez baaz', pinglish_voice, pinglish_engine._frontend, True,
                         '`[.0pA.0ran.1tEz]')):
                    tagged = project((text,), frontend, allow_fallback=False, pinglish=latin).tagged
                    matches = tuple(re.finditer(r'`\[[^]]+\]', tagged))
                    marks = tuple(range(1, len(matches)))
                    proxy = indexed_us_proxy(tagged, en, marks)
                    assert expected in ''.join(p for p in proxy if isinstance(p, str)), (text, proxy)
                    sequence, position = [], 0
                    for i, mark in enumerate(marks, 1):
                        boundary = word_boundary(tagged, matches, i)
                        sequence.extend((tagged[position:boundary], EciIndex(mark)))
                        position = boundary
                    sequence.append(tagged[position:])
                    target = CapturedRows.from_capture(api.capture(carrier, sequence, audio=False))
                    donor = CapturedRows.from_capture(api.capture(en, proxy, audio=False))
                    counts = tuple(len(re.findall(r'\.[012]', m.group())) for m in matches)
                    assert len(target.syllables) == len(donor.syllables) == sum(counts), text
                    assert abs(vowel_start(target, 2) - vowel_start(donor, 2)) <= 15, (text, speed)
                    observed = ObservedApi(api)
                    assert fit_us_word_pitch(target, (0, len(target.pcm)//2), donor,
                                             (0, len(donor.pcm)//2), marks, marks,
                                             SAMPLE_RATES[sample_rate], observed,
                                             syllable_counts=counts), text
                    assert observed.anchored >= 1, (text, counts)

        # Check the whole 0..999 range, not just the originally reported 96.
        # Each numeral's silent pilot must retain its word and syllable count.
        for number in (*map(str, range(1000)), '1996'):
            tagged = project((number,), engine._frontend, allow_fallback=False).tagged
            expected = re.findall(r'`\[([^]]+)\]', tagged)
            proxy = indexed_us_proxy(tagged, en, tuple(range(1, len(expected))))
            displayed = en._phonemes_for_probe(''.join(p for p in proxy if isinstance(p, str)))
            # Invalid no-primary donor tags used to be spelled as many
            # English letter names inside the otherwise silent pitch pilot.
            actual = re.findall(r'`\[([^]]+)\]', displayed)
            counts = lambda values: [len(re.findall(r'\.[012]', value)) for value in values]
            assert counts(expected) == counts(actual), (number, proxy, displayed)
        pinglish = voice._activate(voice._key(PINGLISH_PROBE.id, '1'))
        tags = [project((value,), engine._frontend, allow_fallback=False).tagged
                for value in ('96', '۹۶', '٩٦')]
        assert len(set(tags)) == 1
        assert project(('96',), pinglish._frontend, allow_fallback=False, pinglish=True).tagged == tags[0]

        voice.set_parameter(SAMPLE_RATE, 3)
        voice.set_parameter(NATIVE_SPEED, 65)
        # The provider restarts its local clock at each sentence, sometimes
        # with a non-zero first syllable. Both traces must use the utterance
        # clock, including the final sentence when there is no next mark.
        for text in ('سلام: 96', 'سلام: 86', 'سلام دنیا. امروز این یک آزمایش است. 96'):
            tagged = project((text,), engine._frontend, allow_fallback=False).tagged
            proxy = persian_proxy_for_english(tagged, en, mark_weak=True)
            for carrier, value in ((fa, tagged), (en, proxy)):
                captured = CapturedRows.from_capture(api.capture(carrier, (value,), audio=False))
                assert len(captured.syllables) >= 6, (text, captured.syllables)
                assert all(0 <= a < b <= c < d for (a, b), (c, d) in
                           zip(captured.syllables, captured.syllables[1:])), (text, captured.syllables)
                assert captured.syllables[-1][1] > 10000000, (text, captured.syllables)

        def render(parts):
            sound, marks = bytearray(), []
            assert voice.speak(parts, sound.extend, lambda value: marks.append((value, len(sound))))
            return bytes(sound), marks
        for colon, period in (('سلام دنیا: این یک آزمایش است؟', 'سلام دنیا. این یک آزمایش است؟'),
                              ('NVDA: / دنیا', 'NVDA. / دنیا'),
                              ('سلام (دنیا): خوب', 'سلام (دنیا. خوب')):
            assert render((colon,)) == render((period,)), (colon, period)
        assert render((':',))[0] != render(('.',))[0], 'isolated punctuation lost its name'

        observed = []
        def record(*args, **kwargs):
            args = list(args)
            wrapped = ObservedApi(args[7])
            args[7] = wrapped
            result = fit_us_word_pitch(*args, **kwargs)
            observed.append((wrapped.anchored, kwargs.get('syllable_counts')))
            return result
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=record), \
             patch.object(continuous_plan, 'fit_us_word_pitch', side_effect=record):
            for text in ('96', 'سلام 96 دنیا', '96: سلام', 'سلام: 96'):
                observed.clear()
                assert render((text,))[0]
                assert any(count and 4 in (counts or ()) for count, counts in observed), (text, observed)
                if text == 'سلام: 96':
                    # A prose colon is a complete sentence boundary. Both
                    # sides are independently fitted when streamed early.
                    assert any(count and counts == (2,) for count, counts in observed), (text, observed)
                    assert any(count and counts == (4,) for count, counts in observed), (text, observed)
    finally:
        voice.close()
    print('Passed Persian sentence-colon boundaries, number-96 donor timing and safe number proxies.')


if __name__ == '__main__':
    main()
