"""Check that mixed boundaries carry the native US resonance trajectory."""
from __future__ import annotations

import argparse
from bisect import bisect_left
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, PROCESS_TAGS,
                               SAMPLE_RATE, SAMPLE_RATES)
from _reloq_persian_frames import FrameCapture, FrameReplay
import continuous_plan
from continuous_plan import (bridge_english_continuations, context_plan, english_phone_proxy,
                             english_proxy, _copy_frames, _edges_by_count, _starts)
from text_frontend import SpeechPart, native_tag_accepted


def voice(path, dialect):
    result = DirectEciEngine(path.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    result.set_parameter(PROCESS_TAGS, 1)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    fa, en = voice(args.probe, 0x90000), voice(args.probe, 0x10000)
    scope = fa._dll.reloq_fa_acoustic_scope
    checks = 0
    try:
        assert scope(1) == 0
        # english_proxy constructs these syllables itself; they are not
        # arbitrary source-side phoneme tags. Verify its complete alphabet.
        for stress in '012':
            for vowel in 'Aaeiou':
                tag = f'`[.{stress}m{vowel}]'
                assert native_tag_accepted(tag, fa._phonemes_for_probe(tag)), tag
        for tag in ('`[.1mA.0mi.0me]', '`[.2ma.0mo.1mu.0mi]', '`[.0mi.0mA.1me]'):
            assert native_tag_accepted(tag, fa._phonemes_for_probe(tag)), tag
        for text in ('NVDA', 'hello', 'open settings', 'use Eloquence acoustic timing',
                     'screen reader', 'hello, world', 'unfamiliar transcript'):
            shown = en._phonemes_for_probe(text)
            tags = continuous_plan._converted(shown, continuous_plan._EN_TO_FA)
            if tags and all(native_tag_accepted(tag, fa._phonemes_for_probe(tag)) for tag in tags):
                expected = (continuous_plan._display_punctuation(shown, tags)
                            if re.search(r'[.?!,;:]\s*\S', text)
                            else ' '.join(tags) + continuous_plan._punctuation(text))
            else:
                expected = english_proxy(en, text)
            assert english_phone_proxy(en, fa, text) == expected, text
        assert ')' in english_proxy(en, 'hello) ')
        assert ')' in english_phone_proxy(en, fa, 'hello) ')
        for rate in (3, 6):
            for gender in (0, 1):
                for item in (fa, en):
                    item.set_parameter(SAMPLE_RATE, rate)
                    item.set_parameter(GENDER, gender)
                # Parenthesized language changes have native pauses: keep
                # each language's captured F0 throughout its own segment.
                bracketed = (SpeechPart('en-us', 'hello) '), SpeechPart('fa', '`[.1do.0nja]'))
                own = {}
                fit = continuous_plan._fit_context_pitch
                def observe(item, *items, **options):
                    before = tuple(row.controls[1] for row in item.frames)
                    fit(item, *items, **options)
                    own[items[4]] = before == tuple(row.controls[1] for row in item.frames)
                continuous_plan._fit_context_pitch = observe
                try:
                    bracket_plan = context_plan(bracketed, fa, en, SAMPLE_RATES[rate])
                finally:
                    continuous_plan._fit_context_pitch = fit
                assert own == {'fa': True, 'en-us': True}, own
                assert bracket_plan.pilot is None, 'a separated phrase needs no shared-pitch synthesis'
                assert not continuous_plan._bracket_boundary(
                    (SpeechPart('en-us', 'NVDA '), SpeechPart('fa', '`[.1Gu]')), 1)
                # Observe the original English /vowel/ -> /s/ boundary.
                native = FrameCapture(en._dll).capture(en, ('say ', EciIndex(42), 'see'))
                frames = native.frames()
                tick = native.indexes[0][1] * 1000 / SAMPLE_RATES[rate]
                boundary = bisect_left(_starts(frames), tick)
                assert 1 <= boundary < len(frames)
                for column in (9, 13, 15):
                    assert abs(frames[boundary].controls[column] -
                               frames[boundary - 1].controls[column]) <= 12
                for left, tag in (('say ', '`[.1si]'), ('say ', '`[.1ha]'),
                                  ('go ', '`[.1tA]')):
                    parts = (SpeechPart('en-us', left), SpeechPart('fa', tag))
                    plan = context_plan(parts, fa, en, SAMPLE_RATES[rate])
                    original = _copy_frames(plan.frames)
                    boundary = _edges_by_count(plan.counts)[0]
                    starts = _starts(original)
                    assert bridge_english_continuations(plan, parts, (), ()) == (1,)
                    assert len(plan.frames) == len(original)
                    for index, (before, after) in enumerate(zip(original, plan.frames)):
                        # Never alter the preceding word, source levels,
                        # stop closures or later parts of the new word.
                        for column in range(62):
                            if (index >= boundary and starts[index] - starts[boundary] <= 30 and
                                    column in (9, 13, 15)):
                                continue
                            assert before.controls[column] == after.controls[column], (left, tag, index, column)
                    for column in (9, 13, 15):
                        assert abs(plan.frames[boundary].controls[column] -
                                   original[boundary - 1].controls[column]) < .01
                    pcm = bytearray()
                    assert FrameReplay(fa._dll).replay(fa, plan.frames, (), pcm.extend, lambda _: None)
                    assert pcm
                    checks += 1
    finally:
        scope(0)
        fa.close()
        en.close()
    print(f'Passed {checks} US-guided continuation, source and stop-boundary checks.')


if __name__ == '__main__':
    main()
