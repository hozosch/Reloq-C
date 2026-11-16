"""Check independent contour planning, settings, fallback and cancellation."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import threading
import time

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_driver
from _reloq_eci_direct import EciIndex, EciPitch, EciProsody, GENDER, HEAD_SIZE, NATIVE_SPEED, NATIVE_WPM, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture
from nvda_driver_probe import PERSIAN_PROBE, US_ENGLISH, make_language_engine
from parallel_pilot import ParallelPilot, _CaptureVoice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=option != 'worker')
    args = parser.parse_args()
    old_min = continuous_driver._PARALLEL_MIN
    old_fast_min = continuous_driver._FAST_PILOT_MIN
    old_cpu_count = continuous_driver.os.cpu_count
    continuous_driver.os.cpu_count = lambda: 2
    engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                  args.espeak_library, args.data_parent)
    old_init, old_submit, old_capture, old_speak = (ParallelPilot.__init__, ParallelPilot.submit,
                                                    ParallelPilot._capture, _CaptureVoice.speak)
    old_frame_capture = FrameCapture.capture
    submissions = []
    def submit(self, **kwargs):
        submissions.append(1)
        return old_submit(self, **kwargs)
    ParallelPilot.submit = submit
    def render(parts):
        pcm, marks = bytearray(), []
        assert engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        return bytes(pcm), marks
    def render_serial(parts):
        cpu_count = continuous_driver.os.cpu_count
        continuous_driver.os.cpu_count = lambda: 1
        try:
            return render(parts)
        finally:
            continuous_driver.os.cpu_count = cpu_count
    examples = (
        (EciIndex(1), 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از ', EciIndex(2), 'NVDA', EciIndex(3)),
        (EciIndex(4), EciPitch(75), 'این یک آزمایش تعجب دیگر است و ', EciIndex(5), 'NVDA خوب است.', EciIndex(6)),
        (EciIndex(7), 'NVDA ', EciIndex(8), 'پژوهشگر و دانشگاه خانه دوست', EciIndex(9)),
        (EciProsody(6, 80), 'او آمد و پنجره را باز کرد و ', EciIndex(10), 'window'),
    )
    try:
        switched = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                        args.espeak_library, args.data_parent,
                                        language=US_ENGLISH.id)
        try:
            switched.cancel()
            switched.select_voice(PERSIAN_PROBE.id, '1')
            assert switched._persian_english_prepared
            switched_pcm = bytearray()
            assert switched.speak(examples[0], switched_pcm.extend, lambda _: None) and switched_pcm
        finally:
            switched.close()
        continuous_driver.os.cpu_count = lambda: 1
        single = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                      args.espeak_library, args.data_parent)
        try:
            assert not hasattr(single, '_persian_contour_worker')
            single_pcm = bytearray()
            assert single.speak(examples[0], single_pcm.extend, lambda _: None) and single_pcm
            assert not hasattr(single, '_persian_contour_worker')
        finally:
            single.close()
            continuous_driver.os.cpu_count = lambda: 2
        # Selecting Persian initializes only the extra voice handle, without
        # retaining a previous text, phone sequence, contour or audio.
        assert engine._persian_contour_worker is not None
        assert engine._persian_english_prepared and US_ENGLISH.id in engine._engines
        initial_pcm = bytearray()
        assert engine.speak(examples[0], initial_pcm.extend, lambda _: None)
        assert initial_pcm
        checks = 0
        for rate, gender, head, speed in ((1, 0, 50, 40), (3, 0, 50, 98),
                                          (6, 1, 22, 140), (3, 1, 86, 200)):
            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                               (NATIVE_SPEED, speed)):
                engine.set_parameter(key, value)
            for parts in examples:
                continuous_driver._PARALLEL_MIN = 10**9
                baseline = render_serial(parts)
                continuous_driver._PARALLEL_MIN = 1
                before = len(submissions)
                candidate = render(parts)
                assert candidate == baseline, (rate, gender, head, speed, parts)
                assert len(submissions) > before, 'the independent contour was not used'
                checks += 1

        # A worker failure must fall back before any audio or source index is
        # released, with exactly the same speech as the serial planner.
        continuous_driver._PARALLEL_MIN = 10**9
        expected = render(examples[0])
        continuous_driver._PARALLEL_MIN = 1
        worker_started, main_started = threading.Event(), threading.Event()
        def barrier_worker(voice, parts, on_pcm, on_index):
            worker_started.set()
            if not main_started.wait(5):
                raise RuntimeError('main contour capture did not start concurrently')
            return old_speak(voice, parts, on_pcm, on_index)
        def barrier_main(capture, voice, parts, **kwargs):
            if not kwargs.get('audio', True) and not isinstance(voice, _CaptureVoice):
                assert worker_started.wait(5), 'parallel contour capture did not start'
                main_started.set()
            return old_frame_capture(capture, voice, parts, **kwargs)
        _CaptureVoice.speak, FrameCapture.capture = barrier_worker, barrier_main
        try:
            assert render(examples[0]) == expected and worker_started.is_set() and main_started.is_set()
        finally:
            _CaptureVoice.speak, FrameCapture.capture = old_speak, old_frame_capture
        def fail(_self, *args, **kwargs):
            raise RuntimeError('injected contour worker failure')
        ParallelPilot._capture = fail
        try:
            assert render(examples[0]) == expected
        finally:
            ParallelPilot._capture = old_capture
        def fail_submit(_self, **kwargs):
            raise OSError('injected worker submission failure')
        ParallelPilot.submit = fail_submit
        try:
            assert render(examples[0]) == expected
        finally:
            ParallelPilot.submit = submit

        # A long first segment may use the control-only pilot for its exact
        # initial contour; the audible-EOF pilot is submitted only after a
        # safe prefix has actually reached the caller.
        engine.set_parameter(NATIVE_SPEED, 98)
        continuous_driver._PARALLEL_MIN = 10**9
        expected = render(examples[0])
        continuous_driver._PARALLEL_MIN = 1
        continuous_driver._FAST_PILOT_MIN = 1
        progress, audio = [], bytearray()
        def observe_submit(worker, **kwargs):
            progress.append((kwargs.get('audio', True), len(audio)))
            return old_submit(worker, **kwargs)
        ParallelPilot.submit = observe_submit
        marks = []
        try:
            assert engine.speak(examples[0], audio.extend, lambda value: marks.append((value, len(audio))))
        finally:
            ParallelPilot.submit = submit
        assert (bytes(audio), marks) == expected
        assert len(progress) == 2 and progress[0] == (False, 0) and progress[1][0] and progress[1][1] > 0, progress

        # English openings such as NVDA may start before the full pilot too,
        # provided that their playable prefix meets the separate safety
        # threshold. Compare every sample and source index even when the
        # second capture is deliberately delayed.
        english_first = (EciIndex(11), 'NVDA ', EciIndex(12), 'سلام دنیا و پنجره', EciIndex(13))
        engine.set_parameter(SAMPLE_RATE, 1)
        continuous_driver._FAST_PILOT_MIN = old_fast_min
        continuous_driver._PARALLEL_MIN = 10**9
        english_expected = render_serial(english_first)
        continuous_driver._PARALLEL_MIN = 1
        progress, audio, marks = [], bytearray(), []
        def slow_full_pilot(worker, *args):
            if args[-1] is True:
                time.sleep(.08)
            return old_capture(worker, *args)
        ParallelPilot.submit, ParallelPilot._capture = observe_submit, slow_full_pilot
        try:
            assert engine.speak(english_first, audio.extend, lambda value: marks.append((value, len(audio))))
        finally:
            ParallelPilot.submit, ParallelPilot._capture = submit, old_capture
        assert (bytes(audio), marks) == english_expected
        assert len(progress) == 2 and progress[0] == (False, 0) and progress[1][0], progress
        prefix_ms = 1000 * progress[1][1] / (2 * SAMPLE_RATES[1])
        assert prefix_ms >= continuous_driver._FAST_ENGLISH_PREFIX_MIN_MS - 1, prefix_ms

        # A short English-to-Ach prefix has insufficient safe audio at this
        # speed. It must submit only the complete pilot, not the quick one.
        short_ach = (EciIndex(14), 'NVDA ', EciIndex(15), 'خوب', EciIndex(16))
        continuous_driver._PARALLEL_MIN = 10**9
        short_expected = render_serial(short_ach)
        continuous_driver._PARALLEL_MIN = 1
        progress, audio, marks = [], bytearray(), []
        ParallelPilot.submit = observe_submit
        try:
            assert engine.speak(short_ach, audio.extend, lambda value: marks.append((value, len(audio))))
        finally:
            ParallelPilot.submit = submit
        assert (bytes(audio), marks) == short_expected
        assert len(progress) == 1 and progress[0] == (True, 0), progress

        # A moderate WPM setting uses its own conservative threshold. Very
        # fast WPM and per-utterance speed controls retain the complete pilot.
        for mode in ('wpm', 'fast_wpm', 'inline'):
            engine.set_parameter(NATIVE_WPM, 180 if mode == 'wpm' else 600 if mode == 'fast_wpm' else 0)
            parts = ((EciProsody(6, 70), *english_first) if mode == 'inline' else english_first)
            reference = render_serial(parts)
            progress, audio, marks = [], bytearray(), []
            ParallelPilot.submit = observe_submit
            try:
                assert engine.speak(parts, audio.extend, lambda value: marks.append((value, len(audio))))
            finally:
                ParallelPilot.submit = submit
            assert (bytes(audio), marks) == reference
            if mode == 'wpm':
                assert len(progress) == 2 and progress[0] == (False, 0) and progress[1][0], (mode, progress)
                assert 1000 * progress[1][1] / (2 * SAMPLE_RATES[1]) >= 199, (mode, progress)
            else:
                assert len(progress) == 1 and progress[0] == (True, 0), (mode, progress)
        engine.set_parameter(NATIVE_WPM, 0)
        engine.set_parameter(SAMPLE_RATE, 3)
        continuous_driver._FAST_PILOT_MIN = 1

        # If the second capture fails after audio has started, complete the
        # first source segment and route the remaining words once. Never
        # replay the prefix or lose NVDA's source marks.
        audio, marks, failed = bytearray(), [], []
        def fail_after_prefix(worker, *args):
            if args[-1] is True:
                failed.append(len(audio))
                raise RuntimeError('injected full-pilot failure after prefix')
            return old_capture(worker, *args)
        ParallelPilot._capture = fail_after_prefix
        try:
            assert engine.speak(examples[0], audio.extend, lambda value: marks.append((value, len(audio))))
        finally:
            ParallelPilot._capture = old_capture
        assert failed and failed[0] > 0 and audio
        assert [value for value, _ in marks] == [1, 2, 3] and marks[-1][1] == len(audio)
        assert render(examples[0]) == expected

        # Stop while the other thread is inside its pilot request. Neither
        # stale audio nor a blocked worker may affect the next utterance.
        continuous_driver._FAST_PILOT_MIN = 10**9
        started, done, stale = threading.Event(), threading.Event(), bytearray()
        def blocked(voice, parts, on_pcm, on_index):
            started.set()
            while not voice.cancelled.wait(.01):
                pass
            return False
        _CaptureVoice.speak = blocked
        def stop():
            if started.wait(5):
                engine.cancel()
            done.set()
        stopper = threading.Thread(target=stop, daemon=True)
        stopper.start()
        try:
            assert not engine.speak(examples[0], stale.extend, lambda _: None)
            assert done.wait(5) and started.is_set() and not stale
        finally:
            _CaptureVoice.speak = old_speak
            stopper.join(5)
        assert not stopper.is_alive()
        assert render(examples[0]) == expected

        # Also stop while the native contour synthesizer is inside its PCM
        # callback, not merely before synthesis starts. The owner must not
        # leak a stale prefix, deadlock, or poison the next request.
        started, done, stale = threading.Event(), threading.Event(), bytearray()
        def during_native(voice, parts, on_pcm, on_index):
            def pcm(data):
                started.set()
                if not done.wait(5):
                    raise RuntimeError('native contour stop did not complete')
                on_pcm(data)
            return old_speak(voice, parts, pcm, on_index)
        _CaptureVoice.speak = during_native
        stopper = threading.Thread(target=stop, daemon=True)
        stopper.start()
        try:
            assert not engine.speak(examples[0], stale.extend, lambda _: None)
            assert done.wait(5) and started.is_set() and not stale
        finally:
            _CaptureVoice.speak = old_speak
            stopper.join(5)
        assert not stopper.is_alive()
        assert render(examples[0]) == expected

        # On the early path cancellation may follow a valid emitted prefix;
        # no samples may be emitted after cancellation while its full pilot
        # is running in the other thread.
        continuous_driver._FAST_PILOT_MIN = 1
        started, done, pcm, cancelled_at = threading.Event(), threading.Event(), bytearray(), []
        _CaptureVoice.speak = during_native
        def stop_after_prefix():
            if started.wait(5):
                cancelled_at.append(len(pcm))
                engine.cancel()
            done.set()
        stopper = threading.Thread(target=stop_after_prefix, daemon=True)
        stopper.start()
        try:
            assert not engine.speak(examples[0], pcm.extend, lambda _: None)
            assert done.wait(5) and started.is_set() and cancelled_at and cancelled_at[0] > 0
            assert len(pcm) == cancelled_at[0]
        finally:
            _CaptureVoice.speak = old_speak
            stopper.join(5)
        assert not stopper.is_alive()
        assert render(examples[0]) == expected
    finally:
        continuous_driver._PARALLEL_MIN = old_min
        continuous_driver._FAST_PILOT_MIN = old_fast_min
        continuous_driver.os.cpu_count = old_cpu_count
        ParallelPilot.__init__ = old_init
        FrameCapture.capture = old_frame_capture
        ParallelPilot.submit, ParallelPilot._capture, _CaptureVoice.speak = old_submit, old_capture, old_speak
        engine.close()
    print(f'Passed {checks} serial/parallel PCM and source-index comparisons, failure recovery and cancellation.')


if __name__ == '__main__':
    main()
