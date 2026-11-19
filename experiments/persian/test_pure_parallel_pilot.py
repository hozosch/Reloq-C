"""Verify that parallel Persian and US planning preserve speech and cancellation."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_driver
from _reloq_eci_direct import (EciIndex, EciPitch, EciProsody, GENDER, HEAD_SIZE,
                               NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE)
from _reloq_persian_frames import FrameCapture
from nvda_driver_probe import make_language_engine
from parallel_pilot import ParallelPilot, _CaptureVoice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    previous_stream = os.environ.get('RELOQ_FA_STREAM_PREFIX')
    os.environ['RELOQ_FA_STREAM_PREFIX'] = '0'
    cpu_count = continuous_driver.os.cpu_count
    continuous_driver.os.cpu_count = lambda: 2
    engine = make_language_engine(args.library, args.library, None, Path('/unavailable'), Path('/unavailable'))
    submit, capture_proxy = ParallelPilot.submit_proxy, ParallelPilot._capture_proxy
    probe, capture = _CaptureVoice._phonemes_for_probe, FrameCapture.capture
    samples = (
        'استفاده از تنظیماتِ ذخیره‌شده‌ی فعلی هنگامِ ورود به ویندوز و دیگر صفحاتِ امن (به اِختیاراتِ مدیریتی نیاز دارد)',
        'برای راه‌اندازی رایانه باید این گزینه انتخاب شود.',
        'او آمد و او آمد و پنجره را باز کرد.',
        'من یک کتاب و یک مداد دارم.',
        'یک', 'سلام دنیا', 'پرانتز باز', '۶۶',
    )
    def render(parts, parallel=True):
        continuous_driver.os.cpu_count = (lambda: 2) if parallel else (lambda: 1)
        pcm, marks = bytearray(), []
        assert engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        return bytes(pcm), marks
    try:
        count = 0
        for rate, gender, head, speed, clock in ((1, 0, 50, 40, 0), (3, 0, 50, 82, 0),
                                                 (6, 1, 22, 140, 0), (3, 1, 86, 98, 1)):
            for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                                     (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                engine.set_parameter(parameter, value)
            for text in samples:
                for parts in ((EciIndex(1), text, EciIndex(2)),
                              (EciPitch(75), text, EciPitch(50)),
                              (EciProsody(6, 80), text)):
                    serial = render(parts, False)
                    assert render(parts, True) == serial, (rate, gender, head, speed, clock, parts)
                    count += 1

        parts = (EciIndex(1), samples[0], EciIndex(2))
        expected = render(parts, False)
        worker_started, main_started = threading.Event(), threading.Event()
        def worker_probe(voice, text):
            worker_started.set()
            if not main_started.wait(5):
                raise RuntimeError('Persian capture did not start concurrently')
            return probe(voice, text)
        def main_capture(api, voice, parts, **kwargs):
            if not isinstance(voice, _CaptureVoice) and not kwargs.get('audio', True):
                assert worker_started.wait(5), 'US proxy did not start concurrently'
                main_started.set()
            return capture(api, voice, parts, **kwargs)
        _CaptureVoice._phonemes_for_probe, FrameCapture.capture = worker_probe, main_capture
        try:
            assert render(parts) == expected and worker_started.is_set() and main_started.is_set()
        finally:
            _CaptureVoice._phonemes_for_probe, FrameCapture.capture = probe, capture

        def failed_submit(_worker, **_kwargs):
            raise OSError('injected submission failure')
        ParallelPilot.submit_proxy = failed_submit
        try:
            assert render(parts) == expected
        finally:
            ParallelPilot.submit_proxy = submit
        def failed_capture(_worker, *_args):
            raise RuntimeError('injected worker failure')
        ParallelPilot._capture_proxy = failed_capture
        try:
            assert render(parts) == expected
        finally:
            ParallelPilot._capture_proxy = capture_proxy

        started = threading.Event()
        def blocked(voice, text):
            started.set()
            if not voice.cancelled.wait(5):
                raise RuntimeError('cancellation timed out')
            raise InterruptedError('cancelled')
        _CaptureVoice._phonemes_for_probe = blocked
        def stop():
            assert started.wait(5)
            engine.cancel()
        stopper = threading.Thread(target=stop, daemon=True)
        stopper.start()
        try:
            stale = bytearray()
            assert not engine.speak(parts, stale.extend, lambda _: None) and not stale
        finally:
            _CaptureVoice._phonemes_for_probe = probe
            stopper.join(5)
        assert not stopper.is_alive() and render(parts) == expected
    finally:
        ParallelPilot.submit_proxy, ParallelPilot._capture_proxy = submit, capture_proxy
        _CaptureVoice._phonemes_for_probe, FrameCapture.capture = probe, capture
        continuous_driver.os.cpu_count = cpu_count
        engine.close()
        if previous_stream is None:
            os.environ.pop('RELOQ_FA_STREAM_PREFIX', None)
        else:
            os.environ['RELOQ_FA_STREAM_PREFIX'] = previous_stream
    print(f'Passed {count} exact parallel/serial comparisons, fallback and cancellation.')


if __name__ == '__main__':
    main()
