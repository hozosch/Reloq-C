"""Check native US fricative context at actual Persian/English switches."""
from __future__ import annotations

import argparse
from bisect import bisect_left
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
import continuous_plan
from continuous_plan import _english_transition_proxy, persian_proxy_for_english
from nvda_driver_probe import make_language_engine
from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, NATIVE_SPEED,
                               PROCESS_TAGS, SAMPLE_RATE)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    lib = args.library.resolve()
    en = DirectEciEngine(lib, dialect=0x10000, encoding='utf8', native_voice=True)
    try:
        en.set_parameter(PROCESS_TAGS, 1)
        text = '`[.0Rer.1mez]'
        old = persian_proxy_for_english(text, en)
        assert old == '`[.0mE.1mE]', old
        assert _english_transition_proxy(text, en, final=True) == '`[.0mE.1mEz]'
        assert _english_transition_proxy(text, en) == old
        assert _english_transition_proxy(text + '.', en, final=True) == old + '.'
        assert _english_transition_proxy('`[.1zArd]', en, initial=True) == '`[.1zA]'
        # The US reader drops a coda /h/ in this proxy. Do not pretend that
        # such a donor represents a real H-to-next-sound transition.
        tagged = '`[.0Rer.1meh]'
        assert _english_transition_proxy(tagged, en, final=True) == persian_proxy_for_english(tagged, en)
    finally:
        en.close()

    voice = make_language_engine(lib, lib, None, Path('/unavailable'), Path('/unavailable'))
    original = continuous_plan.match_us_z_onsets
    original_reverse = continuous_plan.match_h_to_us_z
    observed = []
    reverse_observed = []
    def inspect(plan, parts, rate):
        before = [tuple(row.controls) for row in plan.frames]
        counts, marks = plan.counts, plan.source_marks
        selected = original(plan, parts, rate)
        assert plan.counts == counts and plan.source_marks == marks
        if selected:
            bounds = continuous_plan._edges_by_count(counts)
            donor = plan.contexts['en-us']
            for edge in selected:
                at = bounds[edge - 1]
                ref = bisect_left(donor.ticks, round(plan.edges['en-us'][edge] * 1000 / rate))
                assert plan.frames[at].controls[8] == donor.frames[ref].controls[8]
                assert any(plan.frames[i].controls[8] < before[i][8]
                           for i in range(at, min(at + 20, len(plan.frames))))
                for i in range(len(plan.frames)):
                    for column in range(62):
                        if at <= i < min(at + 32, len(plan.frames)) and (column == 8 or 35 <= column <= 42):
                            continue
                        assert plan.frames[i].controls[column] == before[i][column], (i, column)
            observed.extend(selected)
        return selected
    def inspect_reverse(plan, parts, en, rate):
        before = [tuple(row.controls) for row in plan.frames]
        counts, marks = plan.counts, plan.source_marks
        selected = original_reverse(plan, parts, en, rate)
        assert plan.counts == counts and plan.source_marks == marks
        bounds = continuous_plan._edges_by_count(counts)
        for edge in selected:
            at = bounds[edge - 1]
            assert before[at][2] >= 20 and plan.frames[at].controls[2] <= 5
            for i in range(len(plan.frames)):
                for column in range(62):
                    if at <= i < min(at + 91, len(plan.frames)) and column in (2, 7):
                        continue
                    assert plan.frames[i].controls[column] == before[i][column], (i, column)
            reverse_observed.append(edge)
        return selected
    try:
        for rate, gender, speed in ((1, 0, 40), (3, 0, 82), (6, 1, 100)):
            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                voice.set_parameter(key, value)
            for text in ('قرمز heading', 'مصاف غول‌های قرمز heading level 2 link',
                         'heading زرد', 'روح zero', 'روح zebra', 'روح singer'):
                pcm, marks = bytearray(), []
                with patch.object(continuous_plan, 'match_us_z_onsets', new=inspect), \
                     patch.object(continuous_plan, 'match_h_to_us_z', new=inspect_reverse):
                    assert voice.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                                       lambda value: marks.append((value, len(pcm))))
                assert pcm and marks == [(11, 0), (12, len(pcm))], (rate, gender, speed, text, marks)
        assert observed, 'the native US /z/ onset was not exercised'
        assert reverse_observed, 'the actual Persian /h/ to English /z/ was not exercised'
    finally:
        voice.close()
    print(f'Passed US fricative context and onset transfers: {len(observed)} + {len(reverse_observed)}.')


if __name__ == '__main__':
    main()
