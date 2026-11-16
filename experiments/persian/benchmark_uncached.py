"""Measure first playback feed with different texts and cleared result caches.

This uses the actual NVDA transport and a nonblocking test player. It is not a
measurement of the user's sound device, NVDA event dispatch, or eSpeak cold
start. Compare both modes on the same machine and build.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import statistics
import sys
import threading
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_transport import EciTextTransport
from _reloq_persian_frames import FrameStream
from _reloq_eci_direct import EciIndex
from nvda_driver_probe import US_ENGLISH, make_language_engine


MIXED = (
    'NVDA خوب است.', 'سلام window خوب است.', 'hello دنیا', 'computer سلام',
    'music خط', 'system کتاب', 'weather خانه', 'دوست player خانه',
    'office سلام', 'settings کتاب', 'software خوب است.', 'keyboard سلام',
    'NVDA فایل', 'computer شماره', 'سلام browser خوب', 'window سلام دنیا',
)
ENGLISH = ('one simple label', 'this is a test', 'hello world', 'keyboard window',
           'listen to settings', 'system information', 'open the file', 'weather forecast')
FALLBACK_WORDS = ('پژوهشگر', 'پیش‌نمایش', 'همگام‌سازی', 'برگزیده', 'اشتراک‌گذاری',
                  'الکترونیکی', 'پردازشگر')
FALLBACK = tuple(('NVDA ', EciIndex(100 + i), word, EciIndex(200 + i), ' سلام')
                 for i, word in enumerate(FALLBACK_WORDS))


class Player:
    def __init__(self):
        self.first = None
    def feed(self, pcm, onDone=None):
        if pcm and self.first is None:
            self.first = time.perf_counter_ns()
        if onDone:
            onDone()
    def idle(self): pass
    def stop(self): pass
    def pause(self, _switch): pass
    def close(self): pass


class Uncached:
    """Clear cross-request result caches on the actual owning worker."""
    def __init__(self, engine):
        self.engine = engine
    def __getattr__(self, name):
        return getattr(self.engine, name)
    def speak(self, parts, on_pcm, on_index):
        for name in ('_persian_plan_cache', '_persian_phoneme_cache',
                     '_persian_ach_cache', '_uppercase_cache'):
            cache = getattr(self.engine, name, None)
            if cache is not None:
                cache.clear()
        for voice in self.engine._engines.values():
            frontend = getattr(voice, '_frontend', None)
            cache = getattr(frontend, '_resilient_word_cache', None)
            if cache is not None:
                cache.clear()
        return self.engine.speak(parts, on_pcm, on_index)


def measure(args, *, streaming, english=False, fallback=False, cancel_before=False):
    FrameStream.available = staticmethod(args.available if streaming else lambda _dll: False)
    player, done, errors = Player(), threading.Event(), []
    def factory():
        options = {'language': US_ENGLISH.id} if english else {}
        return Uncached(make_language_engine(args.scope_probe, args.native_library, args.worker,
                                            args.espeak_library, args.data_parent, **options))
    transport = EciTextTransport(factory, player, lambda _value: None, lambda _value: done.set(),
                                 lambda error: (errors.append(str(error)), done.set()))
    def run(parts):
        player.first = None
        done.clear()
        start = time.perf_counter_ns()
        transport.submit((parts,) if isinstance(parts, str) else parts)
        if not done.wait(20):
            raise RuntimeError('transport benchmark timed out')
        if errors or player.first is None:
            raise RuntimeError('transport benchmark failed: ' + repr(errors))
        return round((player.first - start) / 1e6, 3)
    try:
        cold = run('one simple label' if english else FALLBACK[0] if fallback else 'NVDA خوب است.')
        texts = ENGLISH if english else FALLBACK if fallback else MIXED
        timings = []
        for text in texts:
            if cancel_before:
                transport.cancel()
            label = ''.join(part if isinstance(part, str) else '' for part in text) if isinstance(text, tuple) else text
            timings.append((label, run(text)))
    finally:
        if not transport.shutdown():
            raise RuntimeError('benchmark transport did not shut down')
    return {'cold_first_feed_ms': cold, 'median_first_feed_ms': round(statistics.median(v for _, v in timings), 3),
            'individual_ms': timings}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    args.available = FrameStream.available
    try:
        result = {'uncached_no_stream': measure(args, streaming=False),
                  'uncached_stream': measure(args, streaming=True),
                  'uncached_fallback': measure(args, streaming=True, fallback=True),
                  'uncached_fallback_after_stop': measure(args, streaming=True, fallback=True, cancel_before=True),
                  'native_english': measure(args, streaming=False, english=True)}
    finally:
        FrameStream.available = staticmethod(args.available)
    result['method'] = ('first fake WavePlayer feed through the real NVDA transport; '
                        'different texts with plan, pronunciation and fallback-word result caches '
                        'cleared before every request; handles warmed after the separate first request; '
                        'fallback cases contain internal source indexes')
    encoded = json.dumps(result, ensure_ascii=False, indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded + '\n', encoding='utf8')
    # Windows build consoles may use a legacy code page even though the
    # measurement file itself is UTF-8.
    print(json.dumps(result, ensure_ascii=True, indent=2))


if __name__ == '__main__':
    main()
