"""Verify short pauses, prompt audio, indexes and cancellation in a streaming probe."""
from __future__ import annotations

import numpy as np
import threading

from mixed_language_probe import shorten_boundary
from streaming_boundary import StreamingBoundary


def run(rate: int) -> int:
    checks = 0
    tone = np.full(rate // 10, 120, dtype='<i2').tobytes()
    quiet = bytes(round(rate * .390) * 2)
    output, indexes = bytearray(), []
    stream = StreamingBoundary(rate, output.extend, lambda i: indexes.append((i, len(output) // 2)))
    stream.audio(tone)
    assert output == tone  # Do not delay speech until the end of the segment.
    stream.index(1)
    stream.audio(quiet[:len(quiet) // 2 // 2 * 2])
    stream.index(2)
    stream.audio(quiet[len(quiet) // 2 // 2 * 2:])
    stream.index(3)
    stream.end_part('NVDA', switch=True)
    expected = shorten_boundary(tone + quiet, rate, 'NVDA')
    assert output == expected
    assert [i for i, _offset in indexes] == [1, 2, 3]
    assert indexes[0][1] == len(tone) // 2
    assert indexes[1][1] == indexes[2][1] == len(expected) // 2
    checks += 5
    for text, stop in (('word,', False), ('stop', True), ('last', False)):
        output.clear()
        stream.audio(tone + quiet)
        stream.end_part(text, switch=text != 'last', final_stop=stop)
        assert output == (shorten_boundary(tone + quiet, rate, text, final_stop=stop)
                          if text != 'last' else tone + quiet)
        checks += 1
    output.clear()
    stream.audio(tone)
    stream.audio(bytes(rate * 2))
    stream.end_part('word', switch=True)
    assert output == tone + bytes(rate * 2)  # Do not erase a long natural pause.
    checks += 1
    output.clear()
    stream.audio(tone)
    stream.audio(quiet)
    stream.index(4)
    stream.cancel()
    stream.end_part('word', switch=True)
    stream.audio(tone)
    assert output == tone and all(i != 4 for i, _ in indexes)
    checks += 1
    reentrant, seen = bytearray(), []
    reentrant_stream = None
    def cancel_in_pcm(chunk: bytes) -> None:
        reentrant.extend(chunk)
        reentrant_stream.cancel()
    reentrant_stream = StreamingBoundary(rate, cancel_in_pcm, seen.append)
    reentrant_stream.audio(tone + quiet)
    reentrant_stream.index(5)
    reentrant_stream.end_part('word', switch=True)
    assert reentrant == tone and not seen
    checks += 1
    return checks


def main() -> None:
    total = sum(run(rate) for rate in (11025, 16000, 48000))
    started, release = threading.Event(), threading.Event()
    emitted = bytearray()
    def blocked_pcm(chunk: bytes) -> None:
        emitted.extend(chunk)
        started.set()
        if not release.wait(3):
            raise RuntimeError('streaming cancellation test timed out')
    stream = StreamingBoundary(16000, blocked_pcm, lambda _: None)
    tone = np.full(1600, 120, dtype='<i2').tobytes()
    worker = threading.Thread(target=stream.audio, args=(tone,))
    worker.start()
    assert started.wait(3)
    canceller = threading.Thread(target=stream.cancel)
    canceller.start()
    canceller.join(1)
    prompt = not canceller.is_alive()
    release.set()
    worker.join(3)
    canceller.join(3)
    assert prompt and not worker.is_alive() and not canceller.is_alive()
    stream.audio(tone)
    assert emitted == tone
    total += 1
    print(f'Passed {total} streaming, index, punctuation and cancellation checks.')


if __name__ == '__main__':
    main()
