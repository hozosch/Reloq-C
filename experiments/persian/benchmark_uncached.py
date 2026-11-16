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
from nvda_driver_probe import US_ENGLISH, make_language_engine


MIXED = (
    'NVDA خوب است.', 'سلام window خوب است.', 'hello دنیا', 'computer سلام',
    'music خط', 'system کتاب', 'weather خانه', 'دوست player خانه',
    'office سلام', 'settings کتاب', 'software خوب است.', 'keyboard سلام',
    'NVDA فایل', 'computer شماره', 'سلام browser خوب', 'window سلام دنیا',
)
ENGLISH = ('one simple label', 'this is a test', 'hello world', 'keyboard window',
           'listen to settings', 'system information', 'open the file', 'weather forecast')


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
        return self.engine.speak(parts, on_pcm, on_index)


def measure(args, *, streaming, english=False):
    FrameStream.available = staticmethod(args.available if streaming else lambda _dll: False)
    player, done, errors = Player(), threading.Event(), []
    def factory():
        options = {'language': US_ENGLISH.id} if english else {}
        return Uncached(make_language_engine(args.scope_probe, args.native_library, args.worker,
                                            args.espeak_library, args.data_parent, **options))
    transport = EciTextTransport(factory, player, lambda _value: None, lambda _value: done.set(),
                                 lambda error: (errors.append(str(error)), done.set()))
    def run(text):
        player.first = None
        done.clear()
        start = time.perf_counter_ns()
        transport.submit((text,))
        if not done.wait(20):
            raise RuntimeError('transport benchmark timed out')
        if errors or player.first is None:
            raise RuntimeError('transport benchmark failed: ' + repr(errors))
        return round((player.first - start) / 1e6, 3)
    try:
        cold = run('one simple label' if english else 'NVDA خوب است.')
        texts = ENGLISH if english else MIXED
        timings = [(text, run(text)) for text in texts]
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
                  'native_english': measure(args, streaming=False, english=True)}
    finally:
        FrameStream.available = staticmethod(args.available)
    result['method'] = ('first fake WavePlayer feed through the real NVDA transport; '
                        'different texts with result caches cleared before every request; '
                        'handles warmed after the separate first request')
    encoded = json.dumps(result, ensure_ascii=False, indent=2)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded + '\n', encoding='utf8')
    print(encoded)


if __name__ == '__main__':
    main()
