"""Check that numbers cannot cut off the following Persian word."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]

from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
import continuous_driver
from continuous_plan import english_phone_proxy
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    voice = make_language_engine(args.library, args.library, None, Path('/unavailable'),
                                 Path('/unavailable'), language=PERSIAN_PROBE.id)
    previous = os.environ.get('RELOQ_FA_STREAM_PREFIX')
    plan_sentence = continuous_driver.context_plan
    observed = []

    def record_plan(parts, *args, **kwargs):
        plan = plan_sentence(parts, *args, **kwargs)
        if len(parts) == 2 and parts[0].language == 'en-us' and parts[1].language == 'fa':
            observed.append((parts, plan))
        return plan

    def render(parts, streaming):
        os.environ['RELOQ_FA_STREAM_PREFIX'] = '1' if streaming else '0'
        pcm, marks = bytearray(), []
        observed.clear()
        with patch.object(continuous_driver, 'context_plan', new=record_plan):
            assert voice.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        assert pcm and len(observed) == 1, (parts, len(observed))
        sentence, plan = observed[0]
        assert sentence[1].text == '`[.0ri.1al]', sentence
        target = plan.contexts['fa']
        # The final word has two syllables. Their native intervals, rather
        # than overall audio length, establish that neither was discarded by
        # the language switch. Times from the provider use 100 ns units.
        assert len(target.syllables) >= 2
        rate = SAMPLE_RATES[voice._engine_parameters[PERSIAN_PROBE.id][SAMPLE_RATE]]
        start, end = target.syllables[-2][0], target.syllables[-1][1]
        first, last = plan.edges['fa'][1:3]
        assert first * 10_000_000 <= start * rate, (parts, first, start, rate)
        assert end * rate <= last * 10_000_000, (parts, end, last, rate)
        assert tuple(mark for mark, _ in marks) == (1, 2, 3), (parts, marks)
        assert marks[0][1] <= marks[1][1] < marks[2][1] == len(pcm), (parts, marks, len(pcm))
        return bytes(pcm), tuple(marks)

    prefixes = (
        'heading    level 3  ۱,۹۰۰,۰۰۰ ',  # Amir's original example
        'heading level 3 1,900,000 ',
        'heading level 3 1,900,000,000 ',
        'heading level 3 1900000 ',
        'heading 1,234 ',
    )
    try:
        checks = 0
        for rate, gender, speed in ((3, 0, 65), (2, 1, 40), (6, 0, 100)):
            for param, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                voice.set_parameter(param, value)
            for prefix in prefixes:
                parts = (EciIndex(1), prefix, EciIndex(2), 'ریال', EciIndex(3))
                full = render(parts, False)
                incremental = render(parts, True)
                assert full == incremental, (rate, gender, speed, prefix)
                checks += 2

        # Punctuation that actually occurs in the English source must remain
        # in the timing proxy. An internally added trailing comma must not.
        fa_key = voice._key(PERSIAN_PROBE.id, '1')
        from nvda_driver_probe import US_ENGLISH
        en_key = voice._key(US_ENGLISH.id, '1')
        fa = continuous_driver._PlanningVoice(voice, fa_key, voice._activate(fa_key))
        en = continuous_driver._PlanningVoice(voice, en_key, voice._activate(en_key))
        assert not english_phone_proxy(en, fa, 'heading level 3 1,900,000 ').endswith(',')
        assert ',' in english_phone_proxy(en, fa, 'one,two,three')
        assert english_phone_proxy(en, fa, 'one,two,three,').endswith(',')
    finally:
        voice.close()
        if previous is None:
            os.environ.pop('RELOQ_FA_STREAM_PREFIX', None)
        else:
            os.environ['RELOQ_FA_STREAM_PREFIX'] = previous
    print(f'Passed {checks} grouped-number/Persian-word boundaries, streaming and indices.')


if __name__ == '__main__':
    main()
