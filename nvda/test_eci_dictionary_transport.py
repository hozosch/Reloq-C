"""Native US dictionary replacement and cancellation through the NVDA queue.

The player simulates a queued feed. No audio device or audio files are used;
this checks worker ownership and UI cancellation, not real NVDA/WASAPI.
"""
from pathlib import Path
import sys
import threading
import time

from _reloq_eci_direct import EciIndex
from _reloq_eci_transport import EciTextTransport
from _reloq_languages import LanguageEngine, US_ENGLISH

ROOT = Path(__file__).resolve().parents[1]


class Player:
    def feed(self, pcm, onDone=None):
        assert pcm
        if onDone:
            onDone()
    def idle(self):pass
    def stop(self):pass
    def pause(self, switch):pass
    def close(self):pass


def main():
    library = ROOT / ("eci/eci.dll" if sys.platform == "win32" else "eci/libreloq_eci.so")
    errors, indices, completed = [], [], []
    ready = threading.Condition()

    def done(number):
        with ready:
            completed.append(number)
            ready.notify_all()

    def wait(number):
        with ready:
            assert ready.wait_for(lambda: number in completed or errors, 5), "Native dictionary queue stalled"
        assert not errors, errors

    transport = EciTextTransport(lambda: LanguageEngine(library, language=US_ENGLISH.id),
                                Player(), indices.append, done, errors.append)
    longest = 0
    try:
        transport.set_dictionary_path(ROOT / "data/community")
        wait(transport.submit(["This is Eloquence and NVDA.", EciIndex(1)]))
        for i in range(12):
            # Control changes survive navigation cancellation and remain on
            # the native owner worker; Cancel runs on the caller thread.
            transport.set_dictionary_path(ROOT / "data/community" if i % 2 else None)
            transport.submit(["This is a longer sentence, with 504 and 4004. " * 20, EciIndex(20 + i)])
            begin = time.perf_counter()
            transport.cancel()
            elapsed = time.perf_counter() - begin
            longest = max(longest, elapsed)
            assert elapsed < 1, f"UI cancellation blocked for {elapsed:.3f} seconds"
        number = transport.submit(["The final sentence is complete, 504 and 4004.", EciIndex(100)])
        wait(number)
        assert indices[-1] == 100
        transport.set_sample_rate(6, lambda rate: Player())
        wait(transport.submit(["A final check at 48 kilohertz.", EciIndex(101)]))
        assert indices[-1] == 101
    finally:
        assert transport.shutdown(timeout=3), "Native close did not finish"
    assert not errors, errors
    print(f"Native dictionary queue: 12 reload/cancel cycles, 48 kHz recovery, maximum Cancel {longest:.6f}s; no audio files")


if __name__ == "__main__":
    main()
