"""Compare the C-owned US pilot with the active Persian sentence planner."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda'), str(ROOT / 'eci')]
from _reloq_eci_direct import PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, NativeFrame
from continuous_driver import _PlanningVoice
from continuous_plan import indexed_us_proxy
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, US_ENGLISH
from test_persian_owned_capture import OwnedCapture


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    owner = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    fa_key = owner._key(PERSIAN_PROBE.id, '1')
    en_key = owner._key(US_ENGLISH.id, '1')
    fa_engine = owner._activate(fa_key)
    en = _PlanningVoice(owner, en_key, owner._activate(en_key))
    en.raw.set_parameter(PROCESS_TAGS, 1)
    capture = FrameCapture(en._dll)
    owned = en._dll.eciPersianCaptureUsProxyUtf8
    owned.argtypes = (ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t,
                      ctypes.c_size_t, ctypes.POINTER(OwnedCapture))
    owned.restype = ctypes.c_int
    release = en._dll.eciPersianReleaseOwnedCapture
    release.argtypes, release.restype = (ctypes.POINTER(OwnedCapture),), None
    texts = ('دو.', 'دو؟', 'سلام دنیا.', 'کتاب را.', 'صفر، یک، دو.',
             'برای راه‌اندازی رایانه باید این گزینه انتخاب شود.')
    checks = 0
    try:
        for rate in range(7):
            owner.set_parameter(SAMPLE_RATE, rate)
            for text in texts:
                tagged = project((text,), fa_engine._frontend, allow_fallback=False).tagged
                count = len(re.findall(r'`\[[^]]+\]', tagged))
                marks = tuple(range(1, count))
                reference = capture.capture(en, indexed_us_proxy(tagged, en, marks), audio=False)
                assert reference is not None
                en._check()
                raw = tagged.encode('utf8'); result = OwnedCapture()
                try:
                    status = owned(en._handle, raw, len(raw), 30000, ctypes.byref(result))
                    assert status == 0, (text, rate, status)
                    data = ctypes.string_at(result.frames, result.frame_count*ctypes.sizeof(NativeFrame))
                    syllables = tuple((result.syllables[i].from_, result.syllables[i].to)
                                      for i in range(result.syllable_count))
                    indexes = tuple((i+1, result.mark_samples[i]) for i in range(result.mark_count))
                    assert data == reference.frame_bytes, (text, rate, 'frames')
                    assert syllables == reference.syllables, (text, rate, 'syllables')
                    assert indexes == reference.indexes, (text, rate, 'indexes')
                    assert result.sample_count == len(reference.pcm)//2, (text, rate, 'clock')
                    checks += 1
                finally:
                    release(ctypes.byref(result))
    finally:
        owner.close()
    print(f'Passed {checks} US pilot comparisons with the existing Persian sentence planner.')


if __name__ == '__main__':
    main()
