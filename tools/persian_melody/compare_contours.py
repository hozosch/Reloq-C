"""Measure how far one Persian sentence melody is from the accepted one.

Renders each sentence through the accepted US-contour path and through the
provider's own melody, tracks the pitch of both recordings and compares them
frame by frame. Both paths use the same phones and durations, so the frames
line up. The summary is the acceptance measure for moving the US melody into
the Persian core: the native melody must come close to the accepted contour
before anyone is asked to listen to it.
"""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import statistics
import sys

RATE = 11025
HOP, WINDOW = int(0.010 * RATE), int(0.040 * RATE)

SENTENCES = (
    'من دیروز کتاب را به علی دادم.', 'امروز هوا خیلی خوب است.', 'او در تهران زندگی می‌کند.',
    'ما فردا به مدرسه می‌رویم.', 'این برنامه برای نابینایان ساخته شده است.',
    'لطفاً در را ببندید و چراغ را خاموش کنید.', 'دیروز با دوستانم به سینما رفتیم و فیلم جدیدی دیدیم.',
    'تنظیمات را باز کنید.', 'پرونده ذخیره شد.', 'سلام، حال شما چطور است؟',
    'شما فردا به تهران می‌روید؟', 'آیا این کتاب را خوانده‌اید؟', 'کجا می‌روی؟', 'چرا دیر آمدی؟',
    'این چیست؟', 'ساعت چند است؟', 'اگر باران ببارد، ما در خانه می‌مانیم.',
    'کتاب، دفتر، مداد و پاک‌کن را بیاور.', 'نه، من نمی‌آیم.', 'بله، درست است.', 'بله... درست است.', 'شاید... نمی‌دانم.', 'سلام (دوست من) خوش آمدی.',
    'چه روز زیبایی!', 'مواظب باش!', 'او گفت که فردا می‌آید.', 'خانهٔ ما نزدیک پارک است.',
    'دانشگاه تهران قدیمی‌ترین دانشگاه ایران است.', 'من زبان فارسی را دوست دارم.',
    'قطار ساعت هشت حرکت می‌کند.', 'هوا سرد شده است، لباس گرم بپوش.',
    'برادرم مهندس است و خواهرم پزشک.', 'این صفحه سه پیوند و دو دکمه دارد.',
)


def load(library, us_rules=False):
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(Path(library).resolve()))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    for name, result, arguments in (
            ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)), ('eciSetParam', I, (H, I, I)),
            ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
            ('eciPersianSpeakNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S), c.POINTER(I), S,
                                             c.POINTER(S))),
            ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                               c.POINTER(I), S, S, c.POINTER(S)))):
        function = getattr(dll, name)
        function.restype, function.argtypes = result, arguments
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)
    handles, keep, sinks = [dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)], [], {}
    assert all(handles)
    for handle in handles:
        buffer, samples = (c.c_int16 * 4096)(), []

        @callback_type
        def receive(_handle, message, count, _context, buffer=buffer, samples=samples):
            if message == 0 and count:
                samples.extend(buffer[:count])
            return 1

        keep.append((buffer, receive))
        sinks[handle] = samples
        dll.eciRegisterCallback(handle, receive, None)
        assert dll.eciSetOutputBuffer(handle, len(buffer), buffer) == 1
        assert dll.eciSetParam(handle, 5, 1) >= 0
    persian, english = handles

    melody = getattr(dll, 'eciPersianNativeMelody', None)
    if us_rules and melody is None:
        raise SystemExit('this library has no US melody rules in the Persian provider')
    if melody is not None:
        melody.argtypes, melody.restype = (I,), I

    def render(kind, text):
        data, error = text.encode('utf8'), S()
        sinks[persian].clear()
        if kind == 'accepted':
            status = dll.eciPersianSpeakUsNativeUtf8(persian, english, data, len(data), 0,
                                                     None, None, 0, 120000, c.byref(error))
        else:
            if us_rules:
                melody(1)
            try:
                status = dll.eciPersianSpeakNativeUtf8(persian, data, len(data), 0, None, None,
                                                       0, c.byref(error))
            finally:
                if us_rules:
                    melody(0)
        sinks[english].clear()
        return (list(sinks[persian]) if status == 0 else None)

    return render, keep


def track(samples):
    """Pitch in hertz per 10 ms frame by autocorrelation; 0 where unvoiced."""
    peak = max((abs(v) for v in samples), default=0) or 1
    low, high, out = RATE // 300, RATE // 60, []
    for start in range(0, len(samples) - WINDOW, HOP):
        frame = samples[start:start + WINDOW]
        mean = sum(frame) / WINDOW
        frame = [v - mean for v in frame]
        energy = sum(v * v for v in frame)
        if (energy / WINDOW) ** 0.5 < 0.04 * peak:
            out.append(0.0)
            continue
        best, lag = 0.0, 0
        for k in range(low, high):
            value = sum(frame[i] * frame[i + k] for i in range(0, WINDOW - k, 2))
            if value > best:
                best, lag = value, k
        out.append(RATE / lag if lag and best * 2 > 0.45 * energy else 0.0)
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--candidate', default='native', choices=('native', 'us-rules'),
                        help="melody to compare with the accepted US contour: the provider's "
                             'own, or the US rules inside the provider')
    parser.add_argument('--list', action='store_true', help='print every sentence')
    parser.add_argument('--check', action='store_true',
                        help='fail unless every sentence keeps its length, the median distance is '
                             'at most 0.3 st and at most 3 %% of the frames are more than 2 st apart')
    args = parser.parse_args()
    render, _keep = load(args.library, us_rules=args.candidate == 'us-rules')
    semitone = lambda a, b: 12 * abs(__import__('math').log2(a / b))
    every, finals, rows = [], [], []
    for text in SENTENCES:
        accepted, candidate = render('accepted', text), render('candidate', text)
        if accepted is None or candidate is None:
            rows.append((text, None))
            continue
        a, b = track(accepted), track(candidate)
        pairs = [(x, y) for x, y in zip(a, b) if x and y]
        if len(pairs) < 10 or abs(len(accepted) - len(candidate)) > RATE // 20:
            rows.append((text, None))
            continue
        diffs = [semitone(x, y) for x, y in pairs]
        every += diffs
        finals += diffs[-30:]
        rows.append((text, (statistics.median(diffs), max(diffs))))
    if args.list:
        for text, value in rows:
            print('%-58s %s' % (text, 'not comparable' if value is None else
                                'median %.2f st, largest %.2f st' % value))
    compared = sum(value is not None for _, value in rows)
    if not every:
        raise SystemExit('no sentence was comparable: the candidate changes durations or fails')
    every.sort()
    finals.sort()
    quantile = lambda values, q: values[min(len(values) - 1, int(q * len(values)))]
    print(f'{compared} of {len(rows)} sentences compared, {len(every)} voiced frames.')
    print('all frames:      median %.2f st, 90%% within %.2f st, largest %.2f st'
          % (quantile(every, .5), quantile(every, .9), every[-1]))
    print('last 300 ms:     median %.2f st, 90%% within %.2f st, largest %.2f st'
          % (quantile(finals, .5), quantile(finals, .9), finals[-1]))
    far = 100 * sum(v > 2 for v in every) / len(every)
    print('frames more than 2 st apart: %.1f %%' % far)
    if args.check and (compared != len(rows) or quantile(every, .5) > 0.3 or far > 3.0):
        raise SystemExit('the candidate melody is too far from the accepted contour')


if __name__ == '__main__':
    main()
