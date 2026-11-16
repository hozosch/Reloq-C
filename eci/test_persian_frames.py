"""Exercise the shared ECI frame API with ordinary native language cores."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, EciIndex, SAMPLE_RATE, GENDER
from _reloq_persian_frames import FrameCapture, FrameReplay, FrameStream


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--standard', action='store_true', help='verify that probe-only exports stay private')
    args = parser.parse_args()
    dll_path = args.library.resolve()
    checks = 0
    for language, text in ((0x10000, 'NVDA says hello.'), (0x40000, 'Guten Tag.')):
        voice = DirectEciEngine(dll_path, dialect=language, encoding='utf8', native_voice=True)
        try:
            capture, replay = FrameCapture(voice._dll), FrameReplay(voice._dll)
            assert capture._begin.__name__ == 'eciPersianCaptureBegin'
            assert replay._play.__name__ == 'eciPersianReplayFrames'
            if args.standard:
                for name in ('reloq_fa_capture_begin', 'reloq_fa_replay_frames',
                             'reloq_fa_acoustic_scope'):
                    assert not hasattr(voice._dll, name), name
            assert FrameStream.available(voice._dll)
            for rate, gender in ((1, 0), (3, 0), (6, 1)):
                voice.set_parameter(SAMPLE_RATE, rate)
                voice.set_parameter(GENDER, gender)
                parts = (EciIndex(1), text, EciIndex(2))
                rendered, rendered_marks = bytearray(), []
                assert voice.speak(parts, rendered.extend,
                                   lambda value: rendered_marks.append((value, len(rendered) // 2)))
                captured = capture.capture(voice, parts)
                # Compare the native PCM path with the former per-frame
                # Python callback, including exact source-index positions.
                storage = capture._store_pcm
                capture._store_pcm = None
                try:
                    callbacks = capture.capture(voice, parts)
                finally:
                    capture._store_pcm = storage
                silent = capture.capture(voice, parts, audio=False)
                assert captured and callbacks and silent and captured.pcm == callbacks.pcm == rendered
                assert captured.frame_bytes == callbacks.frame_bytes and captured.indexes == callbacks.indexes
                assert captured.indexes == tuple(rendered_marks) == silent.indexes
                assert len(silent.pcm) == len(captured.pcm) and silent.frame_bytes == captured.frame_bytes
                rows, replayed, indexes = captured.frames(), bytearray(), []
                assert replay.replay(voice, rows, ((0, 1), (len(rows), 2)), replayed.extend,
                                     lambda value: indexes.append((value, len(replayed) // 2)))
                assert replayed == rendered and indexes == rendered_marks
                middle = len(rows) // 2
                streamed, indexes = bytearray(), []
                stream = FrameStream(voice, streamed.extend,
                                     lambda value: indexes.append((value, len(streamed) // 2)))
                try:
                    assert stream._begin.__name__ == 'eciPersianReplayBegin'
                    stream.append(rows, 0, middle, ((0, 1),))
                    stream.append(rows, middle, len(rows), ((len(rows) - middle, 2),))
                    stream.close(flush=True)
                finally:
                    stream.close()
                assert streamed == rendered and indexes == rendered_marks
                checks += 1
            # Begin/end are request-local and reject nesting and bad bounds.
            assert capture._begin(0) != 0
            assert capture._begin(8) == 0 and capture._begin(8) != 0
            count, failed = ctypes.c_size_t(), ctypes.c_int()
            capture._end(ctypes.byref(count), ctypes.byref(failed))
            capture._release()
            assert count.value == 0 and failed.value == 0
        finally:
            voice.close()

    # Two owning threads can capture separate handles at the same time.
    barrier, errors = threading.Barrier(2), []
    def worker(language, text):
        voice = None
        try:
            voice = DirectEciEngine(dll_path, dialect=language, encoding='utf8', native_voice=True)
            capture = FrameCapture(voice._dll)
            barrier.wait(timeout=10)
            value = capture.capture(voice, (text,))
            assert value and value.pcm and value.frame_bytes
        except BaseException as error:
            errors.append(error)
        finally:
            if voice is not None:
                voice.close()
    threads = (threading.Thread(target=worker, args=(0x10000, 'hello')),
               threading.Thread(target=worker, args=(0x40000, 'hallo')))
    for thread in threads: thread.start()
    for thread in threads: thread.join(timeout=20)
    assert all(not thread.is_alive() for thread in threads) and not errors, errors
    print(f'Passed {checks} shared ECI capture/replay comparisons and two-thread isolation.')


if __name__ == '__main__':
    main()
