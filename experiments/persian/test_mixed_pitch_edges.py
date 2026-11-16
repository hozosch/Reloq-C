"""Check that a connected language switch keeps English word pitch intact."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_persian_frames import FrameCapture, NativeFrame
from continuous_plan import CapturedRows, _fit_context_pitch
from text_frontend import SpeechPart


def make_rows():
    frames = (NativeFrame * 300)()
    for i, row in enumerate(frames):
        row.tick = i
        row.controls[0] = 1.0
        row.controls[1] = 120.0 + (i % 31)
        row.controls[2] = 50.0
        row.controls[9] = 500.0 + i
    return CapturedRows(b'', {}, frames, list(range(300)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path)
    args = parser.parse_args()
    native = None
    if args.library:
        api = FrameCapture(ctypes.CDLL(str(args.library.resolve())))
        native = api.fit_pitch_edges
        assert native is not None, 'native edge fitting is not exported'
    pilot = (ctypes.c_float * 301)(*(200.0 + i / 10 for i in range(301)))
    parts = (SpeechPart('en-us', 'Main landmark'), SpeechPart('fa', 'سلام'))
    bounds = (0, 220, 300)
    pilot_bounds = (0, 220, 300)
    plain, accelerated = make_rows(), make_rows()
    original = bytes(plain.frames)
    _fit_context_pitch(plain, bounds, pilot, pilot_bounds, parts, 'en-us', 1000)
    if native is not None:
        _fit_context_pitch(accelerated, bounds, pilot, pilot_bounds, parts, 'en-us', 1000,
                           native_edges=native)
        assert bytes(plain.frames) == bytes(accelerated.frames)
    before = (NativeFrame * 300).from_buffer_copy(original)
    # The first 175 ms of English are untouched; only its final 45 ms
    # approach the Persian pilot. No unrelated control or frame timing moves.
    for i in range(175):
        assert plain.frames[i].controls[1] == before[i].controls[1], i
    assert plain.frames[219].controls[1] == pilot[220]
    assert plain.frames[200].controls[1] != before[200].controls[1]
    assert all(plain.frames[i].controls[0] == before[i].controls[0] and
               plain.frames[i].controls[9] == before[i].controls[9] for i in range(300))
    # Quick-prefix and full planning may visit the same segment twice.
    once = bytes(plain.frames)
    _fit_context_pitch(plain, bounds, pilot, pilot_bounds, parts, 'en-us', 1000)
    assert bytes(plain.frames) == once
    print('Passed native-English contour preservation, edge blending and repeated planning.')


if __name__ == '__main__':
    main()
