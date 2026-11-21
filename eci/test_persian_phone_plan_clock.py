"""Check Persian phone timing against its planned syllables.

The direct US intonation needs vowel timing, not just a syllable count.
This is a diagnostic for the current acoustic bridge: provider phone
clocks also contain pauses, and a doubled affricate can be assigned to a
different syllable than its language-owned plan. Do not use this test's
simple positional pairing as a general phone alignment algorithm.
"""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import random
import sys

from test_persian_owned_capture import NativeFrame, NativeSyllable
from test_persian_syllable_plan_clock import Event


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S, B = c.c_void_p, c.c_int, c.c_size_t, c.c_ubyte
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_short))),
        ('eciPersianCaptureBegin', I, (S,)), ('eciPersianCaptureSetControlsOnly', I, (I,)),
        ('eciPersianCaptureSetPhoneTrace', I, (I,)),
        ('eciPersianSpeakNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S), c.POINTER(I), S, c.POINTER(S))),
        ('eciPersianAnalyzeRunsCompleteUtf8', I, (c.c_char_p, S, I, c.POINTER(S), c.POINTER(B), S,
            c.POINTER(S), c.POINTER(B), S, c.POINTER(c.POINTER(Event)), c.POINTER(S),
            c.POINTER(S), c.POINTER(S))),
        ('eciPersianRelease', None, (H,)),
        ('eciPersianCaptureEnd', c.POINTER(NativeFrame), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCaptureSyllables', c.POINTER(NativeSyllable), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCaptureRelease', None, ()),
    )
    for name, result, parameters in functions:
        function = getattr(dll, name)
        function.restype, function.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)

    @callback_type
    def callback(_handle, _message, _value, _context):
        return 1

    def layout(raw, variant):
        end, mode, events, count, error = S(len(raw)), B(0), c.POINTER(Event)(), S(), S()
        status = dll.eciPersianAnalyzeRunsCompleteUtf8(
            raw, len(raw), variant, c.byref(end), c.byref(mode), 1, None, None, 0,
            c.byref(events), c.byref(count), None, c.byref(error))
        assert status == 0, (raw, variant, status, error.value)
        try:
            result, syllables, word, previous = [], [], -1, None
            for event in events[:count.value]:
                if event.kind == 1:  # word begin
                    word += 1
                    previous = None
                elif event.kind == 2:  # phone
                    key = (word, event.syllable)
                    if key != previous:
                        syllables.append(key)
                        previous = key
                    result.append((key, event.value))
                elif event.kind == 4 and event.value in (ord(','), 0x060c):
                    result.append((None, 0))  # an internal comma pause
            result.append((None, 0))  # provider's final pause
            return result, syllables
        finally:
            dll.eciPersianRelease(events)

    def capture(voice, raw, variant, phones):
        assert dll.eciPersianCaptureBegin(30000) == 0
        try:
            assert dll.eciPersianCaptureSetControlsOnly(1) == 0
            assert dll.eciPersianCaptureSetPhoneTrace(phones) == 0
            error = S()
            status = dll.eciPersianSpeakNativeUtf8(
                voice, raw, len(raw), variant, None, None, 0, c.byref(error))
            assert status == 0, (raw, variant, status, error.value)
            count, failed = S(), I()
            dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert not failed.value and count.value
            clocks = dll.eciPersianCaptureSyllables(c.byref(count), c.byref(failed))
            assert not failed.value and clocks and count.value
            return tuple((clocks[i].from_, clocks[i].to) for i in range(count.value))
        finally:
            dll.eciPersianCaptureRelease()

    def paired(voice, text, variant):
        raw = text.encode('utf8')
        phones, syllables = layout(raw, variant)
        syllable_clocks = capture(voice, raw, variant, 0)
        phone_clocks = capture(voice, raw, variant, 1)
        assert len(syllables) == len(syllable_clocks), (text, syllables, syllable_clocks)
        assert len(phones) == len(phone_clocks), (text, phones, phone_clocks)
        times = dict(zip(syllables, syllable_clocks))
        mismatches = []
        for (key, identity), (start, end) in zip(phones, phone_clocks):
            assert 0 <= start < end, (text, key, start, end)
            if key is None:
                continue
            first, last = times[key]
            if not (first <= start and end <= last):
                mismatches.append((key, identity, start, end, first, last))
        return mismatches

    words = (
        ('سلام', 'دنیا', 'کتاب', 'را', 'من', 'او', 'آمد', 'سوال', 'خوب',
         'برای', 'گزینه', 'است', 'خانه'),
        ('salaam', 'donyaa', 'ketaab', 'raa', 'man', 'u', 'aamad', 'soal', 'khoob',
         'baraaye', 'gozine', 'ast', 'khoone'),
    )
    rng = random.Random(745)
    corpus = []
    for variant in (0, 1):
        corpus.append([
            rng.choice((' ', ', ', '، ')).join(rng.choice(words[variant])
                for _ in range(rng.randrange(1, 5))) + rng.choice(('', '.', '?'))
            for _ in range(100)
        ])
    checks = 0
    for rate in (0, 1, 3, 6):
        for gender in (0, 1):
            voice = dll.eciNewEx(0x90000)
            assert voice
            output = (c.c_short * 2048)()
            try:
                assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
                dll.eciRegisterCallback(voice, callback, None)
                for parameter, value in ((5, rate), (0x1010, gender),
                    (0x1007, 140 if gender else 85), (0x1008, 75 if gender else 65), (0x100b, 0)):
                    assert dll.eciSetParam(voice, parameter, value) >= 0
                for variant, samples in enumerate(corpus):
                    for text in samples:
                        assert not paired(voice, text, variant), (rate, gender, text)
                        checks += 1
                # Current geminate handling has the same total number of
                # phone clocks, but its J can belong to another syllable.
                # A future contour adapter must resolve or reject it instead
                # of trusting counts alone.
                for variant, text in ((0, 'تعجب.'), (1, 'tajjub.')):
                    mismatch = paired(voice, text, variant)
                    assert mismatch and all(item[1] == 24 for item in mismatch), (rate, gender, text, mismatch)
            finally:
                dll.eciDelete(voice)
    print(f'Passed {checks} Persian phone/syllable clock checks; geminate ambiguity detected for both variants.')


if __name__ == '__main__':
    main()
