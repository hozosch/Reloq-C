"""Measure real Persian/US-English language-switch silence in the probe."""
from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np

from listening import render
from _reloq_eci_direct import EciLanguage, PROCESS_TAGS, SAMPLE_RATES
from _reloq_languages import LanguageEngine
from mixed_language_probe import last_phone_is_stop, shorten_boundary, splice
from streaming_boundary import StreamingBoundary
from test_selected_probe import engine, set_confirmed_persian_probe, SAMPLE_RATE


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    fa = engine(args.probe, 0x90000)
    en = engine(args.probe, 0x10000)
    checks = 0
    try:
        for mode in (1, 3, 6):
            rate = SAMPLE_RATES[mode]
            for e in (fa, en):
                e.set_parameter(SAMPLE_RATE, mode)
            set_confirmed_persian_probe()
            inputs = ((fa, '`[.0mAn]'), (en, 'NVDA'),
                      (fa, "`[.0ra] `[.1d'u:'st] `[.1da.0rAm]."))
            items = [(render(e, text), text, last_phone_is_stop(e._phonemes_for_probe(text)))
                     for e, text in inputs]
            result = splice(items, rate)
            streamed, indexes = bytearray(), []
            joiner = StreamingBoundary(rate, streamed.extend, indexes.append)
            for index, (raw, text, stop) in enumerate(items):
                middle = (len(raw) // 4) * 2
                joiner.audio(raw[:middle])
                joiner.index(index + 1)
                joiner.audio(raw[middle:])
                joiner.end_part(text, switch=index + 1 < len(items), final_stop=stop)
            assert bytes(streamed) == result and indexes == [1, 2, 3], ('streaming mismatch', mode)
            checks += 1
            for raw, text, stop in items[:-1]:
                trimmed = shorten_boundary(raw, rate, text, final_stop=stop)
                assert len(raw) > len(trimmed) + rate // 2, ('insufficient pause removed', mode, text)
                samples = np.frombuffer(trimmed, dtype='<i2').astype(np.int32)
                last = np.flatnonzero(np.abs(samples) > 4)[-1]
                gap_ms = 1000 * (len(samples) - 1 - last) / rate
                assert 24 <= gap_ms <= 26, ('unexpected boundary gap', mode, text, gap_ms)
                checks += 2
            assert result.endswith(items[-1][0]), ('final segment changed', mode)
            checks += 1
        # The comparison's separate captures must reproduce the actual
        # LanguageEngine switch before any pause trimming is applied.
        language_engine = LanguageEngine(args.probe, language=0x90000)
        try:
            language_engine.set_parameter(SAMPLE_RATE, 3)
            language_engine.set_parameter(PROCESS_TAGS, 1)
            sample = [('`[.0mAn]', 0x90000), ('NVDA', 0x10000),
                      ("`[.0ra] `[.1d'u:'st] `[.1da.0rAm].", 0x90000)]
            queue = [sample[0][0], EciLanguage(0x10000, '1'), sample[1][0],
                     EciLanguage(0x90000, '1'), sample[2][0]]
            complete = bytearray()
            assert language_engine.speak(queue, complete.extend, lambda _: None)
            split = bytearray()
            for text, language in sample:
                assert language_engine.speak([EciLanguage(language, '1'), text], split.extend, lambda _: None)
            assert complete == split, 'split comparison differs from actual language-switch speech'
            checks += 1
        finally:
            language_engine.close()
    finally:
        fa.close()
        en.close()
    print(f'Passed {checks} native bilingual boundary checks across 3 rates.')


if __name__ == '__main__':
    main()
