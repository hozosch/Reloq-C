"""Check Hertz pitch through the standard API, ECI tags and real PCM."""
import argparse
import array
import ctypes
import math
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS


def measure_pitch(pcm, sample_rate, expected):
    """Autocorrelate a steady vowel, with sub-sample peak interpolation."""
    samples = array.array("h")
    samples.frombytes(pcm)
    step = max(1, sample_rate // 8000)
    rate = sample_rate / step
    wave = list(samples[:int(sample_rate * 0.25):step])
    mean = sum(wave) / len(wave)
    wave = [value - mean for value in wave]
    assert sum(value * value for value in wave) > 1e7
    first = max(1, int(rate / (expected * 1.12)))
    last = int(rate / (expected * 0.88)) + 1
    scores = []
    for lag in range(first, last + 1):
        left, right = wave[:-lag], wave[lag:]
        dot = sum(a * b for a, b in zip(left, right))
        den = math.sqrt(sum(a * a for a in left) * sum(b * b for b in right))
        scores.append(dot / den if den else 0)
    peak = max(range(len(scores)), key=scores.__getitem__)
    assert 0 < peak < len(scores) - 1 and scores[peak] > 0.6, (expected, scores[peak])
    left, middle, right = scores[peak - 1:peak + 2]
    delta = 0.5 * (left - right) / (left - 2 * middle + right)
    return rate / (first + peak + delta)


def set_hz(e, frequency):
    assert e.lib.eciSetParam(e.h, 8, 1) >= 0  # eciRealWorldUnits
    assert e.lib.eciSetVoiceParam(e.h, 0, 2, frequency) >= 0
    assert e.lib.eciGetVoiceParam(e.h, 0, 2) == frequency


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    e = Engine(args.library, 0x40000)
    try:
        e.lib.eciCopyVoice.argtypes = (ctypes.c_void_p, ctypes.c_int, ctypes.c_int)
        e.lib.eciCopyVoice.restype = ctypes.c_int
        # The ordinary voice API and tag must produce the same actual PCM.
        assert e.lib.eciSetParam(e.h, 0x1009, 0) >= 0
        for code, rate in ((0, 8000), (1, 11025), (3, 16000), (2, 22050),
                           (4, 32000), (5, 44100), (6, 48000)):
            assert e.lib.eciSetParam(e.h, 5, code) >= 0
            for hz in (60, 123, 150, 300, 400):
                set_hz(e, hz)
                api = e.speak("`[.1a]")
                measured = measure_pitch(api, rate, hz)
                assert abs(measured - hz) < max(0.8, hz * 0.004), (rate, hz, measured)
                assert e.lib.eciSetParam(e.h, 0x1008, 65) >= 0
                assert e.speak(f"`vbhz{hz} `[.1a]") == api, (rate, hz)
                checks += 2
        # A difference smaller than a VB step must be audible, not just
        # retained in eciGetVoiceParam's readback.
        assert e.lib.eciSetParam(e.h, 5, 3) >= 0
        set_hz(e, 150)
        raw = e.lib.eciGetParam(e.h, 0x1008)
        pcm150 = e.speak("`[.1a]")
        set_hz(e, 151)
        assert e.lib.eciGetParam(e.h, 0x1008) == raw
        pcm151 = e.speak("`[.1a]")
        assert pcm150 != pcm151
        assert abs(measure_pitch(pcm151, 16000, 151) - 151) < 0.8
        checks += 3
        for invalid in (39, 423, 0, -1):
            assert e.lib.eciSetVoiceParam(e.h, 0, 2, invalid) == -1
            assert e.lib.eciGetVoiceParam(e.h, 0, 2) == 151
            checks += 2
        assert e.lib.eciCopyVoice(e.h, 0, 9)
        assert e.lib.eciSetParam(e.h, 0x1008, 65) >= 0
        assert e.lib.eciCopyVoice(e.h, 9, 0)
        assert e.lib.eciGetVoiceParam(e.h, 0, 2) == 151
        assert e.speak("`[.1a]") == pcm151
        assert e.lib.eciSetParam(e.h, 8, 0) == 1
        assert e.speak("`[.1a]") == pcm151
        assert e.lib.eciSetParam(e.h, 8, 1) == 0
        assert e.lib.eciGetVoiceParam(e.h, 0, 2) == 151
        checks += 8
        # Resetting the contour in text is independent of the saved settings.
        assert e.lib.eciSetParam(e.h, 0x1009, 30) >= 0
        assert e.lib.eciSetParam(e.h, 0x1008, 65) >= 0
        tag = e.speak("`vf0`vbhz150 `[.1a]")
        assert abs(measure_pitch(tag, 16000, 150) - 150) < 0.8
        assert e.lib.eciGetParam(e.h, 0x1009) == 30
        assert e.lib.eciGetParam(e.h, 0x1008) == 65
        checks += 3
        # Switching to Hertz inside an utterance must work from any VB base.
        assert e.lib.eciSetParam(e.h, 0x1009, 0) >= 0
        for baseline in (0, 25, 50, 65, 81, 100):
            assert e.lib.eciSetParam(e.h, 0x1008, baseline) >= 0
            e.add("Mama ")
            assert e.lib.eciInsertIndex(e.h, 17)
            e.add("`vbhz150 `[.1a]")
            pcm = e.finish()
            assert len(e.indices) == 1 and e.indices[0][0] == 17
            measured = measure_pitch(pcm[e.indices[0][1]:], 16000, 150)
            assert abs(measured - 150) < 0.8, (baseline, measured)
            checks += 1
        assert e.lib.eciSetParam(e.h, 0x1008, 50) >= 0
        plain = e.speak("Hieb, Hieb.")
        after_comma = e.speak("Hieb, `vbhz150 Hieb.")
        assert len(after_comma) == len(plain) and after_comma != plain
        checks += 1
        e.tags(0)
        assert e.speak("`vbhz150 Hallo.") != e.speak("Hallo.")
        e.tags(1)
        for text in ("Die Stimme. `l1.0 The voice.",
                     "Die Stimme. `l6.0 这是测试。"):
            set_hz(e, 150)
            api = e.speak(text)
            assert e.lib.eciSetParam(e.h, 0x1008, 65) >= 0
            assert e.speak("`vbhz150 " + text) == api
            checks += 1
        e.lib.eciInsertPitch.argtypes = (ctypes.c_void_p, ctypes.c_int)
        e.lib.eciInsertPitch.restype = ctypes.c_int
        expected = e.speak("`vbhz150 Hallo `vb65 Welt.")
        e.add("`vbhz150 Hallo ")
        assert e.lib.eciInsertPitch(e.h, 65)
        e.add("Welt.")
        assert e.finish() == expected
        checks += 2
    finally:
        e.close()

    # Exercise the language cores and queued changes, including two Hertz
    # values that map to the same ordinary VB value.
    for language, (prefix, word, suffix) in TEXTS.items():
        e = Engine(args.library, language)
        try:
            assert e.lib.eciSetParam(e.h, 5, 3) >= 0
            assert e.lib.eciSetParam(e.h, 0x1009, 0) >= 0
            text = prefix + word + suffix
            set_hz(e, 150)
            api = e.speak(text)
            if language in (0x10000, 0x40000, 0x60000, 0x80000, 0xa0000):
                assert e.lib.eciSetParam(e.h, 0x100a, 50) >= 0
                quiet = e.speak(text)
                assert len(quiet) == len(api)
                loud_samples, quiet_samples = array.array("h", api), array.array("h", quiet)
                ratio = math.sqrt(sum(x*x for x in quiet_samples) / sum(x*x for x in loud_samples))
                assert 0.11 < ratio < 0.15, (hex(language), ratio)
                assert e.lib.eciSetParam(e.h, 0x100a, 100) >= 0
                checks += 1
            assert e.lib.eciSetParam(e.h, 0x1008, 65) >= 0
            assert e.speak("`vbhz150 " + text) == api, hex(language)
            set_hz(e, 150)
            expected = e.speak(prefix + "`vbhz151 " + word + suffix)
            e.tags(0)
            e.add(prefix)
            set_hz(e, 151)
            e.add(word + suffix)
            assert e.finish() == expected, hex(language)
            e.tags(1)
            set_hz(e, 150)
            expected = e.speak(prefix + "`vb65 " + word + suffix)
            e.tags(0)
            e.add(prefix)
            assert e.lib.eciSetParam(e.h, 0x1008, 65) >= 0
            e.add(word + suffix)
            assert e.finish() == expected, hex(language)
            checks += 3
        finally:
            e.close()
    print(f"ECI Hertz pitch: {checks} acoustic, API, tag, queue and language checks passed")


if __name__ == "__main__":
    main()
