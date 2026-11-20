"""Compare early mixed-language replay with the completed sentence plan."""
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
from _reloq_eci_direct import (EciIndex, GENDER, HEAD_SIZE, NATIVE_SPEED,
                               ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE)
from _reloq_persian_frames import FrameStream
from nvda_driver_probe import make_language_engine, PINGLISH_PROBE
from parallel_pilot import ParallelPilot


TEXTS = (
    'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA',
    'پخش کردنِ صدا هنگامِ آغازِ کار و خروج از NVDA',
    'بررسیِ خودکارِ به‌روزرسانیها برای NVDA',
    'پخش کردنِ صدا هنگامِ آغازِ کار و خروج از NVDAبررسیِ خودکارِ به‌روزرسانیها برای NVDA',
    'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)',
    'من NVDA را دوست دارم و با آن کتاب می‌خوانم',
    'برای راه‌اندازی NVDA هنگامِ ورود به ویندوز از این گزینه استفاده کنید',
    'NVDA برای خواندن متن در سیستم عامل ویندوز استفاده می‌شود',
    'سلام دوستان امروز NVDA را برای خواندن کتاب استفاده می‌کنم',
    'متن English در یک جمله فارسی و ادامه سخن',
    'ایران NVDA خوب است',
    'خانه و رایانه جدید NVDA را اجرا می‌کنند',
    'برنامه‌های فارسی با NVDA در ویندوز کار می‌کنند',
    'برای خواندن کتاب با NVDA از صفحه کلید استفاده می‌کنیم',
    'سلام NVDA و دوستان',
    'این یک نمونه متن است، سپس English و ادامه فارسی',
    'به رایانه و اینترنت NVDA نیاز داریم',
    'پرانتز باز NVDA پرانتز بسته',
    'Persian text نقطه txt',
    'Read text نقطه txt',
    'English سلام دوستان',
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    previous = os.environ.get('RELOQ_FA_STREAM_PREFIX')
    engine = make_language_engine(args.library, args.library, None, Path('/unavailable'), Path('/unavailable'))
    original_submit = ParallelPilot.submit_snapshot
    original_append = FrameStream.append
    original_capture = ParallelPilot._capture

    def render(parts, enabled):
        os.environ['RELOQ_FA_STREAM_PREFIX'] = '1' if enabled else '0'
        pcm, marks, captures, genuinely_early = bytearray(), [], [], []
        word_captures, first_word_early = [], []
        def submit(worker, **kwargs):
            future = original_submit(worker, **kwargs)
            if kwargs.get('audio'):
                captures.append(future)
            if kwargs.get('snapshot_at') != 1:
                word_captures.append(future)
            return future
        def append(stream, frames, first, last, marks=()):
            if first == 0 and captures and not captures[-1].done():
                genuinely_early.append(1)
            if first == 0 and len(word_captures) >= 2 and all(not f.done() for f in word_captures[-2:]):
                first_word_early.append(1)
            return original_append(stream, frames, first, last, marks)
        with patch.object(ParallelPilot, 'submit_snapshot', new=submit), \
                patch.object(FrameStream, 'append', new=append):
            ok = engine.speak(parts, pcm.extend, lambda mark: marks.append((mark, len(pcm))))
        assert ok and pcm, parts
        return bytes(pcm), tuple(marks), len(captures), len(genuinely_early), len(first_word_early)

    try:
        compared = early = first_word = 0
        for rate, gender, head, speed, clock in ((1, 0, 50, 40, 0), (3, 0, 50, 82, 0),
                                                 (6, 1, 22, 140, 0), (3, 1, 86, 98, 1)):
            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                               (HEAD_SIZE, head), (NATIVE_SPEED, speed),
                               (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                engine.set_parameter(key, value)
            for text in TEXTS:
                parts = (EciIndex(11), text, EciIndex(12))
                a, b = render(parts, False), render(parts, True)
                assert a[:2] == b[:2], (rate, gender, head, speed, text, len(a[0]), len(b[0]), a[1], b[1])
                early += bool(b[3]); first_word += bool(b[4]); compared += 1
        # Internal NVDA indices must still map to the same audible position.
        for parts in ((EciIndex(21), 'برای راه‌اندازی ', EciIndex(22), 'NVDA در ویندوز', EciIndex(23)),
                      (EciIndex(31), 'سلام ', EciIndex(32), 'NVDA و دوستان', EciIndex(33))):
            assert render(parts, False)[:2] == render(parts, True)[:2], parts
            compared += 1
        engine.select_voice(PINGLISH_PROBE.id, '1')
        for text in ('salam NVDA va donya', 'khane English ketab ra dust daram',
                     'man NVDA ra doost daram'):
            parts = (EciIndex(41), text, EciIndex(42))
            a, b = render(parts, False), render(parts, True)
            assert a[:2] == b[:2], text
            early += bool(b[3]); compared += 1
        assert early >= 12, ('snapshot path was not exercised', early)
        assert first_word >= 8, ('first-word snapshot path was not exercised', first_word)

        # Random direct lexicon words exercise weak words and unpredictable
        # syllable counts without privileging any of the reported phrases.
        engine.select_voice(0x7f0000, '1')
        for key, value in ((SAMPLE_RATE, 3), (GENDER, 0), (HEAD_SIZE, 50),
                           (NATIVE_SPEED, 82), (ORIGINAL_ACOUSTIC_CLOCK, 0)):
            engine.set_parameter(key, value)
        lexicon = Path(__file__).with_name('direct_fa_lexicon.inc').read_text(encoding='utf8')
        words = re.findall(r'^\s*\{"([^"\\]+)",\s*"`\[', lexicon, re.M)
        rng = random.Random(172)
        for i in range(24):
            text = ' '.join(rng.sample(words, 5 + i % 5)) + ' NVDA'
            parts = (EciIndex(61), text, EciIndex(62))
            a, b = render(parts, False), render(parts, True)
            assert a[:2] == b[:2], ('random mixed prefix', i, text)
            compared += 1

        # If the independent target capture fails after its prefix was
        # handed off, retry it and finish without repeating or losing audio.
        retry_parts = (EciIndex(71), TEXTS[0], EciIndex(72))
        expected = render(retry_parts, False)
        failed = []
        def fail_target_once(worker, *args, **kwargs):
            result = original_capture(worker, *args, **kwargs)
            if worker._persian and len(args) > 8 and args[8] is not None and not failed:
                failed.append(True)
                raise RuntimeError('simulated target capture failure after snapshot')
            return result
        with patch.object(ParallelPilot, '_capture', new=fail_target_once):
            actual = render(retry_parts, True)
        assert failed and actual[4] and actual[:2] == expected[:2], (
            'target retry', failed, actual[4], len(actual[0]), len(expected[0]), actual[1], expected[1])

        # Interrupt after the streamed prefix, then prove that the next job
        # is complete and identical to the established non-early path.
        engine.select_voice(0x7f0000, '1')
        os.environ['RELOQ_FA_STREAM_PREFIX'] = '1'
        entered, released = threading.Event(), threading.Event()
        def block(stream, frames, first, last, marks=()):
            result = original_append(stream, frames, first, last, marks)
            if first == 0 and not entered.is_set():
                entered.set()
                if not released.wait(5):
                    raise RuntimeError('cancellation did not release mixed prefix')
            return result
        def cancel():
            if entered.wait(5):
                engine.cancel()
                released.set()
        thread = threading.Thread(target=cancel, daemon=True)
        thread.start()
        with patch.object(FrameStream, 'append', new=block):
            cancelled = engine.speak((TEXTS[0],), lambda _pcm: None, lambda _mark: None)
        thread.join(5)
        assert entered.is_set() and not thread.is_alive() and not cancelled
        assert render((EciIndex(51), 'سلام', EciIndex(52)), True)[:2] == \
               render((EciIndex(51), 'سلام', EciIndex(52)), False)[:2]
    finally:
        engine.close()
        if previous is None:
            os.environ.pop('RELOQ_FA_STREAM_PREFIX', None)
        else:
            os.environ['RELOQ_FA_STREAM_PREFIX'] = previous
    print(f'Passed {compared} mixed early/complete comparisons ({early} genuinely early, '
          f'{first_word} after the first word) and cancellation.')


if __name__ == '__main__':
    main()
