"""Compare incremental Persian playback with the complete US-contour plan."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import random
import re
import sys
import threading
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_driver
from _reloq_eci_direct import (EciIndex, EciPitch, GENDER, HEAD_SIZE, NATIVE_SPEED,
                               ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE)
from _reloq_persian_frames import FrameStream
from nvda_driver_probe import make_language_engine, PINGLISH_PROBE


TEXTS = (
    'استفاده از تنظیماتِ ذخیره‌شده‌ی فعلی هنگامِ ورود به ویندوز و دیگر صفحاتِ امن (به اِختیاراتِ مدیریتی نیاز دارد)',
    'برای راه‌اندازی رایانه باید این گزینه انتخاب شود',
    'من یک کتاب و یک مداد دارم',
    'سلام دوستان، امروز می‌خواهم درباره یک کتاب صحبت کنم',
    'این کتاب درباره دو دوست است',
    'پرانتز باز و پرانتز بسته (سلام دنیا و دوستان)',
    'خواجه‌ئیان و ایران و دانشگاه و مدارس',
    'او آمد و او آمد و من در خانه ماندم',
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    previous = os.environ.get('RELOQ_FA_STREAM_PREFIX')
    cpu_count = continuous_driver.os.cpu_count
    continuous_driver.os.cpu_count = lambda: 2
    os.environ['RELOQ_FA_STREAM_PREFIX'] = '1'
    engine = make_language_engine(args.library, args.library, None, Path('/unavailable'), Path('/unavailable'))
    original_append = FrameStream.append

    def render(parts, enabled):
        os.environ['RELOQ_FA_STREAM_PREFIX'] = '1' if enabled else '0'
        pcm, marks, slices = bytearray(), [], []
        def record(stream, frames, first, last, frame_marks=()):
            slices.append((first, last))
            return original_append(stream, frames, first, last, frame_marks)
        with patch.object(FrameStream, 'append', new=record):
            ok = engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        assert ok and pcm, parts
        return bytes(pcm), marks, slices

    try:
        count = streamed = 0
        settings = ((1, 0, 50, 40, 0), (3, 0, 50, 82, 0),
                    (6, 1, 22, 140, 0), (3, 1, 86, 98, 1))
        for rate, gender, head, speed, clock in settings:
            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                               (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                engine.set_parameter(key, value)
            for text in TEXTS:
                parts = (EciIndex(101), text, EciIndex(102))
                baseline, marks, _ = render(parts, False)
                actual, actual_marks, slices = render(parts, True)
                assert (actual, actual_marks) == (baseline, marks), (rate, gender, head, speed, text)
                if slices:
                    assert len(slices) == 2 and slices[0][0] == 0 and slices[0][1] == slices[1][0]
                    streamed += 1
                count += 1
        assert streamed >= 20, ('most tested sentences did not use incremental replay', streamed)
        # Internal source indexes and pitch commands retain the full-plan path.
        for parts in ((EciIndex(1), 'برای راه‌اندازی ', EciIndex(2), 'رایانه باید این گزینه انتخاب شود', EciIndex(3)),
                      (EciPitch(75), 'برای راه‌اندازی ', EciPitch(50), 'رایانه باید این گزینه انتخاب شود')):
            expected = render(parts, False)
            actual = render(parts, True)
            assert actual[:2] == expected[:2] and not actual[2], parts
            count += 1
        # Sample real lexicon words, including weak words and commas, to
        # detect boundaries whose partial pitch differs from the complete one.
        lexicon = Path(__file__).with_name('direct_fa_lexicon.inc').read_text(encoding='utf8')
        words = re.findall(r'^\s*\{"([^"\\]+)",\s*"`\[', lexicon, re.M)
        rng = random.Random(172)
        for index in range(24):
            for key, value in ((SAMPLE_RATE, 3), (GENDER, index % 2),
                               (HEAD_SIZE, (22, 50, 86)[index % 3]),
                               (NATIVE_SPEED, (40, 82, 140)[index % 3]),
                               (ORIGINAL_ACOUSTIC_CLOCK, 0)):
                engine.set_parameter(key, value)
            selected = rng.sample(words, 5 + index % 8)
            text = ' '.join(selected[:2]) + (', ' if index % 3 == 0 else ' ') + ' '.join(selected[2:])
            parts = (EciIndex(11), text, EciIndex(12))
            a, b = render(parts, False), render(parts, True)
            assert a[:2] == b[:2], ('random', index, text)
            streamed += bool(b[2]); count += 1

        # Cancel after an initial prefix and immediately start a fresh job.
        os.environ['RELOQ_FA_STREAM_PREFIX'] = '1'
        entered, released = threading.Event(), threading.Event()
        def blocked(stream, frames, first, last, frame_marks=()):
            result = original_append(stream, frames, first, last, frame_marks)
            if first == 0:
                entered.set()
                if not released.wait(5):
                    raise RuntimeError('cancellation did not release prefix replay')
            return result
        def stop():
            if entered.wait(5):
                engine.cancel()
                released.set()
        thread = threading.Thread(target=stop, daemon=True)
        thread.start()
        with patch.object(FrameStream, 'append', new=blocked):
            cancelled = engine.speak((TEXTS[1],), lambda _pcm: None, lambda _index: None)
        thread.join(5)
        assert entered.is_set() and not thread.is_alive() and not cancelled
        assert render((EciIndex(41), 'سلام', EciIndex(42)), True)[:2] == \
               render((EciIndex(41), 'سلام', EciIndex(42)), False)[:2]
        engine.select_voice(PINGLISH_PROBE.id, '1')
        for text in ('salam dustan emruz mikhaham dar bare yek ketab sohbat konam',
                     'baraye rah andazi rayane bayad in gozine entekhab shavad',
                     'yek ketab va yek medad daram'):
            parts = (EciIndex(51), text, EciIndex(52))
            a, b = render(parts, False), render(parts, True)
            assert a[:2] == b[:2] and b[2], ('Pinglish', text)
            streamed += 1; count += 1
    finally:
        engine.close()
        continuous_driver.os.cpu_count = cpu_count
        if previous is None:
            os.environ.pop('RELOQ_FA_STREAM_PREFIX', None)
        else:
            os.environ['RELOQ_FA_STREAM_PREFIX'] = previous
    print(f'Passed {count} complete/incremental comparisons ({streamed} streamed) and cancellation.')


if __name__ == '__main__':
    main()
