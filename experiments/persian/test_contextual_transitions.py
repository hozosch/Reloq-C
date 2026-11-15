"""Check phoneme-shaped context and guarded, time-preserving tract bridges."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import contextual_compare, english_phone_proxy, persian_proxy_for_english
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en = engine(args.probe, 0x90000), engine(args.probe, 0x10000)
    checks = 0
    try:
        set_confirmed_persian_probe()
        assert persian_proxy_for_english('`[.0mAn]', en) == '`[.0mAn]'
        assert english_phone_proxy(en, fa, 'NVDA').startswith('`[.1en]')
        assert persian_proxy_for_english('`[.1hw]', en) == persian_proxy_for_english('`[.1hw]')
        checks += 3
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            sentences = ('من NVDA را دوست دارم.', 'آیا NVDA خوب است؟',
                         'من یک laptop دارم.', 'من Windows و NVDA را دوست دارم.',
                         'ده NVDA خوب است؟')
            for mode in (1, 3, 6):
                rate = SAMPLE_RATES[mode]
                for gender in (0, 1):
                    for voice in (fa, en):
                        voice.set_parameter(SAMPLE_RATE, mode)
                        voice.set_parameter(GENDER, gender)
                        voice.set_parameter(NATIVE_PITCH, 81 if gender else 65)
                        voice.set_parameter(NATIVE_INFLECTION, 30)
                    for sentence in sentences:
                        parts = translate_text(api, sentence)
                        plain = contextual_compare(parts, fa, en, args.probe, rate,
                                                   gender=gender, phone_context=True)
                        bridge = contextual_compare(parts, fa, en, args.probe, rate,
                                                    gender=gender, phone_context=True, bridge_ms=25)
                        assert plain.frame_counts == bridge.frame_counts
                        assert len(plain.connected) == len(bridge.connected)
                        assert np.array_equal(plain.controls[:, [0, 1, 2, 3, 4, 5, 6, 7, 8]],
                                              bridge.controls[:, [0, 1, 2, 3, 4, 5, 6, 7, 8]])
                        assert np.array_equal(plain.controls[:, 35:], bridge.controls[:, 35:])
                        assert np.array_equal(plain.controls[plain.controls[:, 8] != 0],
                                              bridge.controls[plain.controls[:, 8] != 0])
                        checks += 5
                        pcm = np.frombuffer(bridge.connected, '<i2').astype(np.int32)
                        assert np.any(pcm) and np.max(np.abs(pcm)) < 32767
                        ticks = np.concatenate(([0.0], np.cumsum(plain.controls[:, 0], dtype=np.float64)))
                        allowed = np.zeros(len(plain.controls), dtype=bool)
                        for index in np.cumsum(plain.frame_counts)[:-1]:
                            allowed |= np.abs(ticks[:-1] - ticks[int(index)]) <= 25
                        assert np.array_equal(plain.controls[~allowed], bridge.controls[~allowed])
                        checks += 2
                        if mode == 3 and gender == 0 and sentence == sentences[0]:
                            old = contextual_compare(parts, fa, en, args.probe, rate)
                            nold = old.frame_counts[0]
                            nnew = plain.frame_counts[0]
                            old_step = abs(old.controls[nold, 9] - old.controls[nold - 1, 9])
                            new_step = abs(plain.controls[nnew, 9] - plain.controls[nnew - 1, 9])
                            bridged_step = abs(bridge.controls[nnew, 9] - bridge.controls[nnew - 1, 9])
                            assert bridge.bridged_edges == (1, 2)
                            assert old_step > 400 and new_step < old_step / 2 and bridged_step < 10
                            checks += 2
                        if mode == 3 and gender == 0 and sentence == sentences[2]:
                            # Preserve the stop closures and burst at yek/laptop.
                            assert bridge.bridged_edges == ()
                            assert np.array_equal(plain.controls, bridge.controls)
                            checks += 2
                        if sentence == sentences[2]:
                            # The accepted E nucleus remains present in yek.
                            end = plain.frame_counts[0]
                            values = plain.controls[:end]
                            target = (495, 2268, 3004) if gender else (450, 1875, 2525)
                            hits = ((values[:, 2] > 20) & (values[:, 9] == target[0]) &
                                    (values[:, 13] == target[1]) & (values[:, 15] == target[2]))
                            assert np.count_nonzero(hits) >= 5, ('accepted E lost', mode, gender)
                            checks += 1
                        if sentence.endswith('؟'):
                            final = parts[-1]
                            trace = bridge.context_after[final.language]
                            bounds = bridge.edges[final.language]
                            start = round(bounds[-2] * 1000 / rate)
                            end = min(round(bounds[-1] * 1000 / rate), len(trace.av))
                            voiced = np.flatnonzero(trace.av[start:end] > 0)
                            first, last = start + int(voiced[0]), start + int(voiced[-1])
                            assert trace.f0[last] > trace.f0[first + (last - first) * 3 // 4]
                            checks += 1
    finally:
        fa.close()
        en.close()
    print(f'Passed {checks} context, transition, source and accepted-voice checks.')


if __name__ == '__main__':
    main()
