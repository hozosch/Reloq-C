"""Exercise NVDA symbol processing and native sentence-context stop joins."""
from __future__ import annotations

import argparse
from bisect import bisect_left
import ctypes
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'nvda'), str(ROOT / 'addon')]
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import NativeFrame
import continuous_driver
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE
from test_persian_virgule import symbol_processor


def values(frames, start, count, columns=(2, 7, 8)):
    return tuple(tuple(round(frames[i].controls[c], 4) for c in columns)
                 for i in range(start, min(start + count, len(frames))))


def check_native_contract(fn):
    # A genuine nasal donor must be accepted; an H donor and overlapping
    # buffers must not change any input, even if the call is malformed.
    target, donor = (NativeFrame * 40)(), (NativeFrame * 40)()
    for frames in (target, donor):
        for row in frames:
            row.controls[0], row.controls[1], row.controls[3] = 1, 650, 60
            row.controls[9], row.controls[13], row.controls[15] = 200, 1500, 2500
    for i in range(10, 40):
        target[i].controls[2] = donor[i].controls[2] = 49
        target[i].controls[7] = 34 if i < 25 else 0
    donor[10].controls[2] = 0
    original = ctypes.string_at(target, ctypes.sizeof(target))
    donor[12].controls[7] = 40
    assert fn(target, 40, 10, 40, donor, 40, 10) == 0
    assert ctypes.string_at(target, ctypes.sizeof(target)) == original
    donor[12].controls[7] = 0
    overlapping = ctypes.cast(ctypes.byref(target, ctypes.sizeof(NativeFrame)), ctypes.POINTER(NativeFrame))
    assert fn(target, 40, 10, 40, overlapping, 39, 10) < 0
    assert ctypes.string_at(target, ctypes.sizeof(target)) == original
    assert fn(target, 40, 10, 40, donor, 40, 10) == 1
    assert target[10].controls[2] == 0 and target[11].controls[7] == 0
    assert target[25].controls[2] == 49 and target[25].controls[7] == 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--transition-shim', type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str((args.transition_shim or args.library).resolve()))
    fn = (getattr(dll, 'eciPersianMatchStopNasal', None) or
          getattr(dll, 'reloq_persian_match_stop_nasal'))
    fn.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_size_t,
                   ctypes.c_size_t, ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_size_t)
    fn.restype = ctypes.c_int
    check_native_contract(fn)
    original, observed = continuous_driver.selected_sentence, []

    def capture(plan, parts, en, german, rate, **kwargs):
        plan._native_match_stop_nasal = fn
        counts = plan.counts
        frames = (NativeFrame * len(plan.frames)).from_buffer_copy(plan.frames)
        edges = [sum(counts[:i]) for i in range(1, len(counts))]
        before = [values(frames, edge, 40) for edge in edges]
        prior = [values(frames, edge - 1, 1) for edge in edges]
        reference = plan.contexts['en-us']
        ref_edges = [bisect_left(reference.ticks, round(plan.edges['en-us'][i] * 1000 / rate))
                     for i in range(1, len(parts))]
        ref_rows = [values(reference.frames, edge, 40) for edge in ref_edges]
        result = original(plan, parts, en, german, rate, **kwargs)
        after_edges = [sum(plan.counts[:i]) for i in range(1, len(plan.counts))]
        after = [values(plan.frames, edge, 40) for edge in after_edges]
        observed.append((parts, counts, plan.counts, before, after, prior, ref_rows))
        return result

    continuous_driver.selected_sentence = capture
    engine = make_language_engine(args.library, args.library, None, Path('/unavailable'),
                                  Path('/unavailable'), language=PERSIAN_PROBE.id)
    processor, level = symbol_processor()
    checked = adopted = preserved = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (50, 65, 95):
                    for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                        engine.set_parameter(parameter, value)
                    for text in ('Persian text.txt', 'text.txt', 'Persian test.txt',
                                 'Persian cap.txt', 'Persian back.txt'):
                        for symbol_level in (level.NONE, level.SOME):
                            processed = processor.processText(text, symbol_level)
                            cut = processed.find(' ')
                            parts = (EciIndex(1), processed[:cut + 1], EciIndex(2),
                                     processed[cut + 1:], EciIndex(3))
                            observed.clear(); pcm = bytearray(); marks = []
                            assert engine.speak(parts, pcm.extend, marks.append)
                            assert pcm and marks == [1, 2, 3], (text, rate, speed, marks)
                            if symbol_level == level.NONE:
                                checked += 1
                                continue
                            assert observed, (text, rate, speed)
                            spoken, old_counts, new_counts, before, after, prior, donor = observed[0]
                            assert len(spoken) == 3 and spoken[0].language == spoken[2].language == 'en-us'
                            assert spoken[1].language == 'fa' and old_counts == new_counts, (text, rate, speed)
                            # A stale aspirated vowel at the first nasal is
                            # replaced by the corresponding US sentence context;
                            # a clean native nasal is never faded a second time.
                            if (prior[0][0] == (0, 0, 0) and
                                    before[0][0][0] > 0 and before[0][0][1] > 0):
                                assert after[0][0] == donor[0][0], (text, rate, speed, before[0][:4], after[0][:4], donor[0][:4])
                                assert any(row[0] > 0 for row in after[0][:5])
                                assert all(row[1] == row[2] == 0 for row in after[0][:8])
                                adopted += 1
                            else:
                                assert after[0][:8] == before[0][:8], (text, rate, speed)
                            # Preserve the voiced lead-in generated by the US
                            # sentence before the stop and its later closure.
                            assert after[1] == before[1], (text, rate, speed, before[1], after[1])
                            assert any(row[0] > 0 for row in after[1][:10])
                            assert any(row == (0, 0, 0) for row in after[1][10:40])
                            preserved += 1; checked += 1
        observed.clear(); pcm = bytearray()
        assert engine.speak(('Persian test هوا txt',), pcm.extend, lambda _: None) and pcm
        _, _, _, before, after, _, _ = observed[0]
        assert any(row[1] > 0 and row[0] == 0 for row in before[0][:30])
        assert before[0][:30] == after[0][:30]
        assert adopted and preserved, (adopted, preserved)
    finally:
        continuous_driver.selected_sentence = original
        engine.close()
    print(f'Passed {checked} actual NVDA symbol-level cases; {adopted} nasal corrections, '
          f'{preserved} native stop lead-ins, H and source indexes.')


if __name__ == '__main__':
    main()
