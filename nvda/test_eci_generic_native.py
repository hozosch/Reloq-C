"""Native ECI smoke check of the opt-in German prose mode and controls.

Run after make -C eci: python3 -B nvda/test_eci_generic_native.py
The Windows package still needs a hearing test in NVDA.
"""

from __future__ import annotations

from pathlib import Path
import threading

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, GENERIC_RATE,
                               GENERIC_PITCH, GENERIC_VOLUME, NATIVE_PITCH)
from _reloq_eci_transport import EciTextTransport


ROOT = Path(__file__).resolve().parents[1]


def main():
    engine = DirectEciEngine(
        ROOT / "eci" / "libreloq_eci.so", experimental_generic=True,
        lexicon_path=ROOT / "data" / "german_apple_observed.rlex")
    try:
        lengths = []
        for rate in (50, 80):
            engine.set_parameter(GENERIC_RATE, rate)
            engine.set_parameter(GENERIC_PITCH, 50)
            engine.set_parameter(GENERIC_VOLUME, 100)
            chunks, marks = [], []
            assert engine.speak(["Heute sprechen wir von Deutschland.",
                                 EciIndex(73)], chunks.append, marks.append)
            assert marks == [73] and chunks and all(len(pcm) % 2 == 0 for pcm in chunks)
            lengths.append(sum(len(pcm) // 2 for pcm in chunks))
        assert 0 < lengths[1] < lengths[0], lengths

        # Native interior marks must use the same whole-sentence acoustic
        # plan. A mark just before or after a separator reaches the same PCM
        # boundary, without resetting intonation or adding playback silence.
        engine.set_parameter(GENERIC_RATE, 50)
        baseline = []
        assert engine.speak(["Heute sprechen wir von Deutschland."],
                            baseline.append, lambda index: None)
        marked, positions = [], []
        assert engine.speak(["Heute sprechen wir", EciIndex(20), " ",
                             EciIndex(21), "von Deutschland.", EciIndex(22)],
                            marked.append,
                            lambda index: positions.append(
                                (index, sum(map(len, marked)))))
        assert b"".join(marked) == b"".join(baseline)
        assert [value for value, _ in positions] == [20, 21, 22]
        assert 0 < positions[0][1] == positions[1][1] < positions[2][1]
        assert positions[2][1] == len(b"".join(marked))

        # Uppercase spelling uses a temporary native pitch mark inside one
        # utterance. It must not split the sentence or affect its neighbours
        # or the next job on the same ECI handle.
        engine.set_parameter(NATIVE_PITCH, 50)
        sentence = ["Peter ", EciIndex(30), "packt ", EciIndex(31),
                    "Pakete.", EciIndex(32)]
        pitched_sentence = ["Peter ", EciIndex(30), EciPitch(85),
                            "packt ", EciIndex(31), EciPitch(50),
                            "Pakete.", EciIndex(32)]

        def render(parts):
            pcm, indices = [], []
            assert engine.speak(parts, pcm.append,
                                lambda value: indices.append(
                                    (value, sum(map(len, pcm)))))
            return b"".join(pcm), indices

        unpitched_pcm, unpitched_marks = render(sentence)
        pitched_pcm, pitched_marks = render(pitched_sentence)
        assert pitched_marks == unpitched_marks
        assert len(pitched_pcm) == len(unpitched_pcm)
        marks_by_value = dict(unpitched_marks)
        first, second = marks_by_value[30], marks_by_value[31]
        assert 0 < first < second < len(pitched_pcm)
        assert pitched_pcm[:first] == unpitched_pcm[:first]
        assert pitched_pcm[first:second] != unpitched_pcm[first:second]
        assert render(sentence) == (unpitched_pcm, unpitched_marks)

        # The NVDA driver inserts a letter boundary when its character mode
        # changes pitch. Verify the single-letter path acoustically too.
        plain_letters, _ = render(["A b."])
        raised_letter, _ = render([EciPitch(85), "A", EciPitch(50), " b."])
        assert len(plain_letters) == len(raised_letter)
        assert raised_letter != plain_letters
    finally:
        engine.close()

    class Player:
        def __init__(self):
            self.pcm = []
            self.callbacks = []
            self.stops = 0
            self.cv = threading.Condition()
            self.played = 0
            self.idle_started = threading.Event()

        def feed(self, pcm, onDone=None):
            def played():
                try:
                    if onDone is not None:
                        onDone()
                finally:
                    with self.cv:
                        self.played += 1
                        self.cv.notify_all()
            with self.cv:
                self.pcm.append(pcm)
                self.callbacks.append(played)

        def idle(self):
            self.idle_started.set()
            with self.cv:
                assert self.cv.wait_for(lambda: self.played == len(self.callbacks), 5)

        def stop(self):
            self.stops += 1
            with self.cv:
                self.played = len(self.callbacks)
                self.cv.notify_all()

        def pause(self, _switch):
            pass

        def close(self):
            pass

    finished = threading.Event()
    notified_done = threading.Event()
    player = Player()
    observed = []
    errors = []

    class Native(DirectEciEngine):
        def speak(self, parts, on_pcm, on_index):
            try:
                return super().speak(parts, on_pcm, on_index)
            finally:
                finished.set()

    transport = EciTextTransport(
        lambda: Native(ROOT / "eci" / "libreloq_eci.so",
                       experimental_generic=True,
                       lexicon_path=ROOT / "data" / "german_apple_observed.rlex"),
        player,
        lambda index: observed.append(("index", index)),
        lambda job: (observed.append(("done", job)), notified_done.set()),
        errors.append)
    try:
        number = transport.submit(["Heute sprechen wir", EciIndex(20), " ",
                                   EciIndex(21), "von Deutschland.", EciIndex(22)])
        assert finished.wait(5), errors
        assert player.idle_started.wait(5), errors
        assert not errors and not observed
        assert b"".join(player.pcm) == b"".join(marked)
        played = 0
        total = sum(map(len, player.pcm))
        for pcm, callback in zip(player.pcm, player.callbacks):
            played += len(pcm)
            assert callback is not None
            callback()
            expected = [("index", value) for value, offset in positions
                        if offset <= played]
            if played == total:
                assert notified_done.wait(1)
                expected.append(("done", number))
            assert observed == expected, (played, observed, expected)
    finally:
        assert transport.shutdown()
    print("generic ECI native: German sentence, interior pitch/index, next-job reset and rate OK")


if __name__ == "__main__":
    main()
