"""Compare Persian plan syllables and source boundaries to the native clock.

The fixed US contour needs the actual Persian syllable times and stress,
without constructing and synthesizing a second donor utterance. This checks
that source-word marks select the same ordered syllables in both the
language-owned plan and the current acoustic provider for simple phrases.
Internal punctuation is included in the syllable-count checks, but its
word-boundary clock needs separate handling. It does not assert that one
source word corresponds to one internal pronunciation group.
"""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import random
import sys

from test_persian_owned_capture import OwnedCapture


class Event(c.Structure):
    _fields_ = (('kind', c.c_uint32), ('value', c.c_uint32), ('flags', c.c_uint32),
                ('syllable', c.c_uint32), ('stress', c.c_uint32))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S, B = c.c_void_p, c.c_int, c.c_size_t, c.c_ubyte
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)),
        ('eciPersianAnalyzeRunsCompleteUtf8', I, (c.c_char_p, S, I, c.POINTER(S), c.POINTER(B), S,
            c.POINTER(S), c.POINTER(B), S, c.POINTER(c.POINTER(Event)), c.POINTER(S),
            c.POINTER(S), c.POINTER(S))),
        ('eciPersianCaptureNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S), S, S,
                                            c.POINTER(OwnedCapture), c.POINTER(S))),
        ('eciPersianRelease', None, (H,)),
        ('eciPersianReleaseOwnedCapture', None, (c.POINTER(OwnedCapture),)),
    )
    for name, result, parameters in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = result, parameters

    words = (
        ('سلام', 'دنیا', 'کتاب', 'را', 'من', 'او', 'آمد', 'تعجب', 'سوال', 'خوب',
         'برای', 'گزینه', 'است', 'خانه'),
        ('salaam', 'donyaa', 'ketaab', 'raa', 'man', 'u', 'aamad', 'tajjub', 'soal',
         'khoob', 'baraaye', 'gozine', 'ast', 'khoone'),
    )
    rng = random.Random(1027)
    corpus = []
    for variant in (0, 1):
        samples = [' '.join(rng.choice(words[variant]) for _ in range(rng.randrange(2, 6))) +
                   rng.choice(('', '.', '?')) for _ in range(120)]
        samples.extend(('سوال خانه?', 'سلام دنیا، خوب است؟') if variant == 0 else
                       ('soal khoone?', 'salaam donyaa, khoob ast?'))
        corpus.append(samples)

    rates = (8000, 11025, 22050, 16000, 32000, 44100, 48000)
    checks = 0
    timed_checks = 0
    for rate_code in (0, 1, 3, 6):
        rate = rates[rate_code]
        for gender in (0, 1):
            voice = dll.eciNewEx(0x90000)
            assert voice
            try:
                for param, value in ((5, rate_code), (0x1010, gender),
                                     (0x1007, 140 if gender else 85), (0x100b, 0)):
                    assert dll.eciSetParam(voice, param, value) >= 0
                for variant, samples in enumerate(corpus):
                    for text in samples:
                        raw = text.encode('utf8'); end = S(len(raw)); mode = B(0); error = S()
                        pieces = text.split(' '); locations = []; byte_count = 0
                        for part in pieces[:-1]:
                            byte_count += len((part + ' ').encode('utf8'))
                            locations.append(byte_count)
                        offsets = (S * len(locations))(*locations)
                        is_index = (B * len(locations))(*([1] * len(locations)))
                        event_marks = (S * len(locations))()
                        events, count = c.POINTER(Event)(), S()
                        status = dll.eciPersianAnalyzeRunsCompleteUtf8(
                            raw, len(raw), variant, c.byref(end), c.byref(mode), 1,
                            offsets, is_index, len(locations), c.byref(events), c.byref(count),
                            event_marks, c.byref(error))
                        assert status == 0, (variant, text, 'analysis', status, error.value)
                        try:
                            boundaries = (0, *event_marks, count.value)
                            syllables = []
                            for first, last in zip(boundaries, boundaries[1:]):
                                previous, stress, size = None, None, 0
                                for event in events[first:last]:
                                    if event.kind == 1:
                                        previous, stress = None, None
                                    elif event.kind == 2:
                                        assert event.stress <= 2
                                        if event.syllable != previous:
                                            if previous is not None:
                                                assert event.syllable == previous + 1
                                            previous, stress = event.syllable, event.stress
                                            size += 1
                                        else:
                                            assert event.stress == stress
                                    elif event.kind == 3:
                                        previous, stress = None, None
                                syllables.append(size)
                        finally:
                            dll.eciPersianRelease(events)

                        capture = OwnedCapture()
                        status = dll.eciPersianCaptureNativeUtf8(
                            voice, raw, len(raw), variant, offsets, len(locations), 30000,
                            c.byref(capture), c.byref(error))
                        try:
                            assert status == 0, (variant, text, 'capture', status, error.value)
                            assert capture.mark_count == len(locations)
                            assert sum(syllables) == capture.syllable_count, (
                                variant, text, syllables, capture.syllable_count)
                            samples_at_words = (0, *capture.mark_samples[:capture.mark_count],
                                                capture.sample_count)
                            at = 0
                            check_boundaries = ',' not in text and '،' not in text
                            for word, number in enumerate(syllables):
                                for _ in range(number):
                                    clock = capture.syllables[at]
                                    assert 0 <= clock.from_ < clock.to
                                    # Clocks are 1/10000 ms. Compare the syllable
                                    # midpoint to word marks without rounding.
                                    midpoint = (clock.from_ + clock.to) * rate
                                    if check_boundaries:
                                        assert samples_at_words[word] * 20000000 <= midpoint < \
                                               samples_at_words[word + 1] * 20000000, (
                                                   variant, text, word, at, clock.from_, clock.to,
                                                   samples_at_words[word:word + 2], rate)
                                    at += 1
                            checks += 1
                            timed_checks += check_boundaries
                        finally:
                            dll.eciPersianReleaseOwnedCapture(c.byref(capture))
            finally:
                dll.eciDelete(voice)
    print(f'Passed {checks} Persian syllable-count checks and {timed_checks} source-word clock checks.')


if __name__ == '__main__':
    main()
