"""Compare captured frame replay with native PCM and test cooperative stop."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import (
    BREATHINESS, EQ_LOW, EciIndex, GENDER, HEAD_SIZE, MELLOW_MODE,
    NATIVE_VOLUME, ROUGHNESS, SAMPLE_RATE, DirectEciEngine, PROCESS_TAGS,
)
from _reloq_persian_frames import FrameCapture, FrameReplay


def engine(library, dialect):
    voice = DirectEciEngine(library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    voice.set_parameter(PROCESS_TAGS, 1)
    return voice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    for dialect, text in ((0x90000, '`[.0sA.1lam] `[.1do.0nja]?'),
                          (0x10000, 'NVDA says hello.'), (0x40000, 'Guten Tag.')):
        voice = engine(args.probe, dialect)
        scope = voice._dll.reloq_fa_acoustic_scope
        scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
        try:
            for rate in (1, 3, 6):
                for gender, head, rough, breath, volume, mellow, eq in (
                        (0, 50, 0, 0, 100, 0, 12),
                        (1, 22, 8, 35, 68, 1, 15),
                        (0, 86, 12, 12, 82, 0, 9)):
                    for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                                             (ROUGHNESS, rough), (BREATHINESS, breath),
                                             (NATIVE_VOLUME, volume), (MELLOW_MODE, mellow), (EQ_LOW, eq)):
                        voice.set_parameter(parameter, value)
                    previous = scope(int(dialect == 0x90000))
                    try:
                        captured = FrameCapture(voice._dll).capture(voice, (EciIndex(8), text, EciIndex(9)))
                    finally:
                        assert scope(previous) == int(dialect == 0x90000)
                    assert captured and captured.pcm
                    rows = captured.frames()
                    audio, indexes = bytearray(), []
                    assert FrameReplay(voice._dll).replay(
                        voice, rows, ((0, 8), (len(rows), 9)), audio.extend,
                        lambda index: indexes.append((index, len(audio) // 2)))
                    assert captured.indexes == tuple(indexes), (dialect, rate, gender, captured.indexes, indexes)
                    assert captured.pcm == audio, (dialect, rate, gender, head, len(captured.pcm), len(audio))
                    checks += 1
        finally:
            voice.close()

    # Cancellation before entry must be detected by the native epoch, even
    # if a reset would otherwise make the session appear active again.
    voice = engine(args.probe, 0x10000)
    try:
        captured = FrameCapture(voice._dll).capture(voice, ('hello',))
        rows = captured.frames()
        replay = FrameReplay(voice._dll)
        old_epoch = replay._epoch(voice._handle)
        voice.cancel()
        assert replay._play(voice._handle, old_epoch, rows, len(rows), None, 0) == 1
        result = bytearray()
        assert replay.replay(voice, rows, (), result.extend, lambda _: None)
        assert result == captured.pcm
    finally:
        voice.close()

    # A native replay in progress must respond to a stop on another thread,
    # then allow a clean next utterance on the owner thread.
    started, release = threading.Event(), threading.Event()
    shared, outcomes = {}, []
    def worker():
        voice = engine(args.probe, 0x10000)
        shared['voice'] = voice
        try:
            rows = FrameCapture(voice._dll).capture(voice, ('testing cancellation',)).frames()
            replay = FrameReplay(voice._dll)
            def receive(_pcm):
                started.set()
                if not release.wait(5):
                    raise RuntimeError('replay cancellation test timed out')
            outcomes.append(replay.replay(voice, rows, (), receive, lambda _: None))
            pcm = bytearray()
            outcomes.append(replay.replay(voice, rows, (), pcm.extend, lambda _: None) and bool(pcm))
        finally:
            voice.close()
    thread = threading.Thread(target=worker)
    thread.start()
    assert started.wait(10)
    stop = threading.Thread(target=shared['voice'].cancel)
    stop.start();stop.join(1)
    prompt = not stop.is_alive()
    release.set();thread.join(10);stop.join(10)
    assert prompt and not thread.is_alive() and outcomes == [False, True], outcomes
    print(f'Passed {checks} replay, voice, equalizer, index and cancellation comparisons.')


if __name__ == '__main__':
    main()
