"""Check a control-only mixed pilot against the complete audible pilot."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import threading
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciPitch, EciText, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
import continuous_driver
import continuous_plan
from nvda_driver_probe import make_language_engine
from parallel_pilot import ParallelPilot


TEXTS = (
    'English سلام دوستان',
    'NVDA سلام دنیا',
    'text نقطه',
    'NVDA برای خواندن متن در ویندوز',
    'سلام NVDA و دوستان',       # Persian-first: keep the existing streaming path.
    'Persian text نقطه txt',   # English-final: keep its audible pilot.
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    # The corpus uses directly covered Persian words; no external frontend is
    # required to verify the shared native contour and playback paths.
    with patch('os.cpu_count', return_value=4):
        engine = make_language_engine(args.library, args.library, None, Path('/unavailable'), Path('/unavailable'))
        original_plan = continuous_driver.context_plan
        original_submit = ParallelPilot.submit
        original_capture = FrameCapture.capture

        def render(parts, enabled, *, force_word_fallback=False):
            pcm, marks, pilots, captures = bytearray(), [], [], []
            def plan(*args, **kwargs):
                kwargs['control_only_pilot'] = enabled
                return original_plan(*args, **kwargs)
            def submit(worker, **kwargs):
                pilots.append(kwargs.get('audio'))
                return original_submit(worker, **kwargs)
            def capture(api, voice, items, **kwargs):
                captures.append(kwargs.get('audio', True))
                return original_capture(api, voice, items, **kwargs)
            with patch.object(continuous_driver, 'context_plan', new=plan), \
                    patch.object(ParallelPilot, 'submit', new=submit), \
                    patch.object(FrameCapture, 'capture', new=capture):
                if force_word_fallback:
                    with patch.object(continuous_plan, 'fit_us_word_pitch', return_value=False):
                        complete = engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
                else:
                    complete = engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))) )
            assert complete and pcm, parts
            return bytes(pcm), tuple(marks), tuple(pilots), tuple(captures)

        count = optimized = 0
        try:
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for speed in (40, 82, 140):
                        for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                            engine.set_parameter(key, value)
                        for text in TEXTS:
                            parts = (EciIndex(1), text, EciIndex(2))
                            baseline = render(parts, False)
                            candidate = render(parts, True)
                            assert baseline[:2] == candidate[:2], (rate, gender, speed, text)
                            if text.startswith(('English ', 'NVDA ', 'text ')):
                                assert False in candidate[2], (text, 'control-only pilot was not selected')
                                optimized += 1
                            count += 1
            # Embedded positions and pitch controls must survive unchanged.
            for parts in ((EciPitch(58), EciIndex(11), 'English ', EciIndex(12), 'سلام دنیا', EciIndex(13)),
                          (EciIndex(21), 'English ', EciText('سلام دنیا', segmentation_guarded=True), EciIndex(22))):
                assert render(parts, False)[:2] == render(parts, True)[:2], parts
                count += 1
            # If a native word fit cannot be validated, re-render the audible
            # pilot and restore the exact ordinary contour before playback.
            parts = (EciIndex(31), 'English سلام دوستان', EciIndex(32))
            baseline = render(parts, False, force_word_fallback=True)
            candidate = render(parts, True, force_word_fallback=True)
            assert baseline[:2] == candidate[:2] and False in candidate[2] and True in candidate[3]
            # Cancellation during the control-only capture must not emit a
            # stale prefix or poison the next request.
            entered, released = threading.Event(), threading.Event()
            def blocked_submit(worker, **kwargs):
                future = original_submit(worker, **kwargs)
                if kwargs.get('audio') is False:
                    entered.set()
                    if not released.wait(5):
                        raise RuntimeError('cancel did not release the pilot')
                return future
            def cancel():
                if entered.wait(5):
                    engine.cancel()
                    released.set()
            thread = threading.Thread(target=cancel, daemon=True)
            thread.start()
            interrupted = bytearray()
            with patch.object(ParallelPilot, 'submit', new=blocked_submit):
                complete = engine.speak(('English سلام دوستان',), interrupted.extend, lambda _value: None)
            thread.join(5)
            assert entered.is_set() and not thread.is_alive() and not complete and not interrupted
            assert render((EciIndex(41), 'سلام', EciIndex(42)), True)[:2] == \
                   render((EciIndex(41), 'سلام', EciIndex(42)), False)[:2]
        finally:
            engine.close()
    print(f'Passed {count} exact mixed pilot comparisons ({optimized} optimized), audible fallback and cancellation.')


if __name__ == '__main__':
    main()
