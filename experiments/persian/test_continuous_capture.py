"""Compare bounded thread-local frame capture with the accepted file probe."""
from __future__ import annotations

import argparse
import ctypes
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import sys
import tempfile

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_persian_frames import FrameCapture, NativeFrame
from listening import render
from shared_sentence_contour import capture
from test_selected_probe import engine, set_confirmed_persian_probe
from _reloq_eci_direct import EciIndex, GENDER, HEAD_SIZE, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--reference', required=True, type=Path)
    args = parser.parse_args()
    assert ctypes.sizeof(NativeFrame) == 280
    cases = ((0x90000, '`[.0sA.1lam] `[.1do.0nja]?'),
             (0x10000, 'NVDA hello?'), (0x40000, '`[.1\'aj\'x.0u]'))
    checks = 0
    for dialect, text in cases:
        voice, reference = engine(args.probe, dialect), engine(args.reference, dialect)
        scope = voice._dll.reloq_fa_acoustic_scope
        scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
        try:
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for target in (voice, reference):
                        target.set_parameter(SAMPLE_RATE, rate)
                        target.set_parameter(GENDER, gender)
                        target.set_parameter(HEAD_SIZE, 50)
                    set_confirmed_persian_probe(d=2, z=2)
                    assert scope(1 if dialect == 0x90000 else 0) == 0
                    try:
                        plain = render(voice, text)
                        actual = FrameCapture(voice._dll).capture(voice, (EciIndex(17), text, EciIndex(18)))
                    finally:
                        assert scope(0) == (1 if dialect == 0x90000 else 0)
                    assert actual and actual.pcm == plain
                    assert [value for value, _ in actual.indexes] == [17, 18]
                    assert actual.indexes[0][1] == 0 and actual.indexes[1][1] == len(plain) // 2
                    rows = actual.frames()
                    assert rows and all(row.head == 50 and row.gender == gender for row in rows)
                    with tempfile.TemporaryDirectory(prefix='fa-frame-parity-') as folder:
                        path = Path(folder)
                        old = capture(reference, [text], path / 'trace', frames=path / 'frames')
                    assert old.pcm == plain, (dialect, rate, gender)
                    expected = old.frames
                    assert len(rows) == len(expected), (dialect, rate, gender, len(rows), len(expected))
                    ticks = np.asarray([row.tick for row in rows])
                    controls = np.asarray([list(row.controls) for row in rows], dtype=np.float32)
                    assert np.array_equal(ticks, expected['tick'])
                    assert np.array_equal(controls, expected['controls']), (dialect, rate, gender)
                    checks += 1
        finally:
            voice.close()
            reference.close()

    # An overlong capture must stop rather than keep allocating, clear its
    # state after failure, and permit a subsequent synthesis on that thread.
    voice = engine(args.probe, 0x10000)
    try:
        capture_api = FrameCapture(voice._dll)
        try:
            capture_api.capture(voice, ('testing',), maximum=5)
            raise AssertionError('oversized capture succeeded')
        except RuntimeError:
            pass
        assert capture_api.capture(voice, ('ok',), maximum=10000)
    finally:
        voice.close()

    def worker(text):
        item = engine(args.probe, 0x10000)
        try:
            result = FrameCapture(item._dll).capture(item, (text,))
            return result.pcm, len(result.frames())
        finally:
            item.close()
    with ThreadPoolExecutor(max_workers=2) as pool:
        results = list(pool.map(worker, ('first.', 'second.')))
    assert all(pcm and count for pcm, count in results) and results[0] != results[1]
    print(f'Passed {checks} frame, PCM, index, limit and concurrency comparisons.')


if __name__ == '__main__':
    main()
