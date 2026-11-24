"""Check the NVDA route that uses the sentence melody fixed in the core.

With the package switch on, ordinary Persian text is spoken by one call into
the core: no US donor voice is synthesized and no contour is aligned in
Python. Text the single call does not take (other scripts, symbols) still
goes through the planner, but that planner no longer asks for a donor
either. The paragraph from issue 49 stopped after a few words because the
donor alignment failed in the middle of a stream; it must now be read to its
end.
"""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
import continuous_driver
import continuous_plan
import nvda_driver_probe
from _reloq_eci_direct import EciIndex

ISSUE_49 = (
    'Photo, ✅میکس VIP  امشب  بصورت رایگان‼️               باضریب⬅️2.46📊 گذاشتماینجا حتما استفاده کنید💎 '
    '💵خودم 500دلار زدم که میشه 1323دلار🔥                                 ⬇️👇⬇️            '
    '🚫فرم VIP رایگان شرطبندی ↙️◀️رفقا اینجا(آرون تیپ) عضو گیری محدوده 👇🅰r3 @ARON_TIP @ARON_TIP  @ARON_TIP\n'
    '            💎💯   100درصد وینه💯💎, Received at 1:20 PM. Viewed 5,338 times. '
    '3 people reacted with ❤  30 of 32')
SENTENCES = ('سلام دنیا.',
             'امروز هوا خیلی خوب است و ما به پارک می‌رویم؛ بعد به خانه برمی‌گردیم.',
             'ساعت چند است؟',
             'کتاب، دفتر، مداد و پاک‌کن را بیاور.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    library = args.library.resolve()
    dll = ctypes.CDLL(str(library))
    select = dll.eciPersianNativeMelody
    select.argtypes, select.restype = (ctypes.c_int,), ctypes.c_int
    engine = nvda_driver_probe.make_language_engine(
        library, library, None, None, None,  # as the NVDA adapter does
        language=nvda_driver_probe.PERSIAN_PROBE.id)

    def speak(parts):
        samples, indexes = [0], []
        def pcm(data):
            samples[0] += len(data) // 2
        result = engine.speak(parts, pcm, indexes.append)
        return result, samples[0], indexes

    try:
        previous = {text: speak([text]) for text in SENTENCES}
        assert all(result is True and count for result, count, _ in previous.values())

        assert select(1) == 0
        continuous_plan.CORE_MELODY = nvda_driver_probe.CORE_MELODY = True
        planned = []
        original = nvda_driver_probe.try_continuous
        def observe(*a, **k):
            planned.append(a[1])
            return original(*a, **k)
        nvda_driver_probe.try_continuous = observe
        captures = []
        capture = continuous_driver.FrameCapture.capture
        def count_captures(self, voice, *a, **k):
            captures.append(voice)
            return capture(self, voice, *a, **k)
        continuous_driver.FrameCapture.capture = count_captures
        try:
            for text in SENTENCES:
                result, count, _ = speak([text])
                assert result is True, text
                # Phones and durations are the core's own in both routes.
                assert count == previous[text][1], (text, count, previous[text][1])
            assert not planned, 'ordinary Persian text must not reach the planner'
            assert not captures, 'ordinary Persian text must not be captured for planning'

            # Index marks arrive, in order, from the single call.
            result, count, indexes = speak(['سلام ', EciIndex(7), 'دنیا، ', EciIndex(8), 'خوب است.',
                                            EciIndex(9)])
            assert result is True and indexes == [7, 8, 9], indexes

            # A word the core reads as English inside Pinglish text keeps one
            # melody across the switch: it goes to the planner as a language
            # switch, and no pause of a sentence end separates the two words.
            pinglish = nvda_driver_probe.make_language_engine(
                library, library, None, None, None,
                language=nvda_driver_probe.PINGLISH_PROBE.id)
            try:
                planned.clear()
                chunks = []
                assert pinglish.speak(['my office'], chunks.append, lambda _mark: None) is True
                assert planned, 'a language switch inside a sentence belongs to the planner'
                pcm = b''.join(chunks)
                values = [abs(int.from_bytes(pcm[i:i + 2], 'little', signed=True))
                          for i in range(0, len(pcm) - 1, 2)]
                quiet = longest = 0
                for start in range(550, len(values) - 3300, 110):
                    if sum(values[start:start + 110]) < 110 * 40:
                        quiet += 1
                        longest = max(longest, quiet)
                    else:
                        quiet = 0
                assert longest < 25, f'{longest * 10} ms of silence inside "my office"'
                planned.clear()
                assert pinglish.speak(['in office khub ast.'], lambda _pcm: None, lambda _mark: None) is True
                assert not planned
            finally:
                pinglish.close()

            result, count, _ = speak([ISSUE_49])
            assert result is True, 'the paragraph from issue 49 must be read to its end'
            assert count > 30 * 11025, count
        finally:
            nvda_driver_probe.try_continuous = original
            continuous_driver.FrameCapture.capture = capture
            continuous_plan.CORE_MELODY = nvda_driver_probe.CORE_MELODY = False
            assert select(0) == 1
    finally:
        engine.close()
    print('Passed the NVDA route with the melody fixed in the core.')


if __name__ == '__main__':
    main()
