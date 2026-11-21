"""The shared C contour planner must preserve accepted Persian audio."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, NATIVE_INFLECTION, NATIVE_PITCH, SAMPLE_RATE
from nvda_driver_probe import make_language_engine
import continuous_driver
import continuous_plan


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    voice = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    voice.set_parameter(SAMPLE_RATE, 3)
    voice.set_parameter(NATIVE_PITCH, 65)
    voice.set_parameter(NATIVE_INFLECTION, 30)
    texts = (
        'برای راه‌اندازی رایانه باید این گزینه انتخاب شود.',
        'سلام دنیا.', 'کتاب را.', 'خوب است.', 'صفر، یک، دو.', 'قدر.',
        'این (یک) مثال است؛ درست؟', 'استفاده از NVDA هنگام ورود به ویندوز.',
        'نمایشِ اِتِّفاقاتِ ثبت‌شده (Log)', 'خواجه‌ئیان', 'تعجب',
        'دو؟', '66',
    )
    def speak(text):
        audio, marks = bytearray(), []
        cut = len(text) // 2
        parts = (EciIndex(11), text[:cut], EciIndex(12), text[cut:], EciIndex(13))
        assert voice.speak(parts, audio.extend, lambda mark: marks.append((mark, len(audio))))
        assert audio and [x for x, _ in marks] == [11, 12, 13]
        return bytes(audio), tuple(marks)
    original_fit = continuous_plan.fit_us_word_pitch
    def old_clock_fit(*args, **kwargs):
        capture = args[7]
        previous = capture.transfer_us_samples
        capture.transfer_us_samples = None
        try:
            return original_fit(*args, **kwargs)
        finally:
            capture.transfer_us_samples = previous
    try:
        # Keep the target and donor on the same thread in both versions, so
        # the comparison covers the planner and not worker scheduling.
        with patch.object(continuous_driver.os, 'cpu_count', return_value=1):
            for text in texts:
                native = speak(text)
                with patch.object(continuous_driver, 'indexed_us_proxy',
                                  continuous_plan._indexed_us_proxy_python), \
                     patch.object(continuous_plan, 'indexed_us_proxy',
                                  continuous_plan._indexed_us_proxy_python), \
                     patch.object(continuous_driver, 'fit_us_word_pitch', old_clock_fit), \
                     patch.object(continuous_plan, 'fit_us_word_pitch', old_clock_fit):
                    previous = speak(text)
                assert native == previous, (text, len(native[0]), len(previous[0]),
                                            native[1], previous[1])
        print(f'Passed {len(texts)} exact Persian audio and NVDA index comparisons.')
    finally:
        voice.close()

if __name__ == '__main__':
    main()
