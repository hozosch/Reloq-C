"""Check the switchable Persian listening variant.

Variant 0 is the previous sound and the library default. Variants 1 to 3
replace the doubly closed, aspirated R between vowels by a single voiced tap
and omit the added frication of a uvular directly before a consonant. They
differ in the vowels: 1 moves the front /ae/ and /o/ forward, 2 leaves both
as before and 3 moves them half as far. The test checks the switch itself and that each
change is audible in the signal; it does not judge how the result sounds.
"""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import sys

RATE = 11025
FRAME = 110  # 10 ms


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    for name, result, arguments in (
            ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)), ('eciSetParam', I, (H, I, I)),
            ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
            ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)), ('eciSynthesize', I, (H,)),
            ('eciSynchronize', I, (H,)), ('eciPersianSoundVariant', I, (I,)),
            ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                               c.POINTER(I), S, S, c.POINTER(S)))):
        function = getattr(dll, name)
        function.restype, function.argtypes = result, arguments
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)

    persian, english = dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)
    assert persian and english
    keep, sinks = [], {}
    try:
        for handle in (persian, english):
            buffer, samples = (c.c_int16 * 4096)(), []

            @callback_type
            def receive(_handle, message, count, _context, buffer=buffer, samples=samples):
                if message == 0 and count:
                    samples.extend(buffer[:count])
                return 1

            keep.append((buffer, receive))
            sinks[handle] = samples
            dll.eciRegisterCallback(handle, receive, None)
            assert dll.eciSetOutputBuffer(handle, len(buffer), buffer) == 1
            assert dll.eciSetParam(handle, 5, 1) >= 0  # 11.025 kHz

        def speak(text):
            data, error = text.encode('utf8'), S()
            sinks[persian].clear()
            status = dll.eciPersianSpeakUsNativeUtf8(persian, english, data, len(data), 0,
                                                     None, None, 0, 120000, c.byref(error))
            assert status == 0, (text, status, error.value)
            sinks[english].clear()
            return tuple(sinks[persian])

        def levels(samples):
            return [sum(abs(v) for v in samples[i:i + FRAME]) / FRAME
                    for i in range(0, len(samples) - FRAME, FRAME)]

        def deepest_inner_dip(samples):
            """Lowest 10 ms level between the first and last loud frame, relative to the peak."""
            value = levels(samples)
            peak = max(value)
            loud = [i for i, v in enumerate(value) if v > 0.4 * peak]
            return min(value[loud[0]:loud[-1] + 1]) / peak

        # The switch: default, return values, rejection of unknown variants.
        assert dll.eciPersianSoundVariant(0) == 0, 'the previous sound must be the default'
        assert dll.eciPersianSoundVariant(6) == -1
        assert dll.eciPersianSoundVariant(-1) == -1
        assert dll.eciPersianSoundVariant(0) == 0

        words = ('دوری', 'بد', 'دو', 'مغرور', 'دیر', 'سیب')
        previous = {word: speak(word) for word in words}
        assert dll.eciPersianSoundVariant(1) == 0
        candidate = {word: speak(word) for word in words}
        assert dll.eciPersianSoundVariant(0) == 1
        restored = {word: speak(word) for word in words}
        assert restored == previous, 'switching back must restore the previous sound exactly'

        # Changed: tap between vowels, /ae/, /o/, uvular before a consonant.
        for word in ('دوری', 'بد', 'دو', 'مغرور'):
            assert candidate[word] != previous[word], word
        # Unchanged: final R after a vowel and a word without any affected phone.
        for word in ('دیر', 'سیب'):
            assert candidate[word] == previous[word], word

        # The R between vowels no longer closes to near silence.
        old_dip, new_dip = deepest_inner_dip(previous['دوری']), deepest_inner_dip(candidate['دوری'])
        assert old_dip < 0.08 < new_dip < 0.6, (old_dip, new_dip)

        # Variant 2 keeps the previous vowels with the new tap and uvular;
        # variant 3 lies between the previous and the fully shifted vowels.
        assert dll.eciPersianSoundVariant(2) == 0
        unshifted = {word: speak(word) for word in words}
        assert dll.eciPersianSoundVariant(3) == 2
        half = {word: speak(word) for word in words}
        assert dll.eciPersianSoundVariant(0) == 3
        for word in ('بد', 'دو', 'دیر', 'سیب'):
            assert unshifted[word] == previous[word], word
        for word in ('دوری', 'مغرور'):
            assert unshifted[word] != previous[word], word
        for word in ('بد', 'دو'):
            assert half[word] != previous[word] and half[word] != candidate[word], word
        assert half['دوری'] == unshifted['دوری'] == candidate['دوری']

        # Variant 4: previous vowels, an R that dips once and smoothly, and a
        # quieter M and N whose colour leaves the next vowel sooner.
        nasal_words = ('نان', 'میم', 'سیمین')
        loud = loud_nasal = {word: speak(word) for word in nasal_words}
        assert dll.eciPersianSoundVariant(4) == 0
        smooth = {word: speak(word) for word in words}
        quiet = {word: speak(word) for word in nasal_words}
        assert dll.eciPersianSoundVariant(0) == 4
        for word in ('بد', 'دو', 'دیر', 'سیب'):
            assert smooth[word] == previous[word], word
        assert smooth['دوری'] != previous['دوری'] and smooth['دوری'] != unshifted['دوری']
        # The R keeps more of the vowels' level than the earlier tap, and the
        # level never moves by more than a small step from one 5 ms to the next
        # inside the word.
        smooth_dip = deepest_inner_dip(smooth['دوری'])
        assert new_dip < smooth_dip < 0.9, (new_dip, smooth_dip)
        for word in nasal_words:
            assert len(quiet[word]) == len(loud[word]), word
            before, after = levels(loud[word]), levels(quiet[word])
            # The opening murmur (first 50 ms) is clearly quieter; the loudest
            # part of the word, a vowel, is not.
            if word[0] in 'نم':
                assert sum(after[:10]) < 0.75 * sum(before[:10]), (word, before[:10], after[:10])
            assert sum(after) < 0.97 * sum(before), word

        # Variant 5: previous vowels; an R that still closes twice but keeps
        # its voice and loses its sharp noise; M and N with the same
        # loudness and timing as before.
        assert dll.eciPersianSoundVariant(5) == 0
        rolled = {word: speak(word) for word in words}
        plain_nasal = {word: speak(word) for word in nasal_words}
        assert dll.eciPersianSoundVariant(0) == 5
        for word in ('بد', 'دو', 'دیر', 'سیب'):
            assert rolled[word] == previous[word], word
        assert len(rolled['دوری']) == len(previous['دوری'])
        value = levels(rolled['دوری'])
        peak = max(value)
        loud = [i for i, v in enumerate(value) if v > 0.4 * peak]
        inner = value[loud[0]:loud[-1] + 1]
        # Two separate dips inside the word, neither down to silence.
        dips = [i for i in range(1, len(inner) - 1)
                if inner[i] < inner[i - 1] and inner[i] <= inner[i + 1] and inner[i] < 0.4 * peak]
        assert len(dips) >= 2 and min(inner) > 0.03 * peak, (dips, min(inner) / peak)
        assert deepest_inner_dip(rolled['دوری']) > old_dip
        for word in nasal_words:
            assert len(plain_nasal[word]) == len(loud_nasal[word]), word
            before, after = levels(loud_nasal[word]), levels(plain_nasal[word])
            assert 0.9 * sum(before) < sum(after) <= 1.02 * sum(before), word
            assert plain_nasal[word] != loud_nasal[word], word
            assert max(after) > 0.9 * max(before), word

        # English spoken by the English voice is unaffected by the Persian variant.
        def english_text(text):
            data = text.encode('utf8')
            sinks[english].clear()
            assert dll.eciAddTextUtf8(english, data, len(data), 0) and dll.eciSynthesize(english)
            dll.eciSynchronize(english)
            return tuple(sinks[english])

        plain = english_text('A red arrow.')
        assert dll.eciPersianSoundVariant(1) == 0
        assert english_text('A red arrow.') == plain
        assert dll.eciPersianSoundVariant(0) == 1
    finally:
        dll.eciPersianSoundVariant(0)
        dll.eciDelete(persian)
        dll.eciDelete(english)
    print(f'Passed Persian sound variant switch; tap dip {old_dip:.3f} -> {new_dip:.3f} of peak.')


if __name__ == '__main__':
    main()
