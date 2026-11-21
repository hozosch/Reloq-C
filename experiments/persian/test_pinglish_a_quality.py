"""Check explicit Pinglish a/aa quality and shared native Persian paths."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import NATIVE_SPEED, SAMPLE_RATE
from direct_fa_sequence import project
from hybrid_fa_frontend import DirectFirstPersian
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine
from test_fa_cluster_final_timing import projector


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    native = dll.eciPersianPinglishWord
    native.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    native.restype = ctypes.c_int
    persian = projector(dll)

    def word(value):
        raw = value.encode('ascii')
        result = ctypes.create_string_buffer(4096)
        status = native(raw, len(raw), result, len(result))
        assert status == 1, (value, status)
        return result.value.decode('ascii')

    pairs = (
        ('bar', 'baar', '`[.1bAr]', '`[.1bar]'),
        ('man', 'maan', '`[.1mAn]', '`[.1man]'),
        ('bad', 'baad', '`[.1bAd]', '`[.1bad]'),
        ('kam', 'kaam', '`[.1kAm]', '`[.1kam]'),
        ('kar', 'kaar', '`[.1kAr]', '`[.1kar]'),
        ('aks', 'aaks', '`[.1Aks]', '`[.1aks]'),
        ('baran', 'baaraan', '`[.0bA.1rAn]', '`[.0ba.1ran]'),
        ('parantez', 'paraantez', '`[.0pA.0rAn.1tez]', '`[.0pA.0ran.1tez]'),
        ('tanzimat', 'tanzimaat', '`[.0tAn.0zi.1mAt]', '`[.0tAn.0zi.1mat]'),
    )
    frontend = DirectFirstPersian(args.library, None, None, None)
    try:
        for short, back, short_tag, back_tag in pairs:
            for spelling, expected in ((short, short_tag), (back, back_tag)):
                assert word(spelling) == expected, (spelling, word(spelling), expected)
                assert word(spelling.upper()) == expected, spelling
                projected = project((spelling,), frontend, allow_fallback=False, pinglish=True)
                assert projected.tagged == expected, (spelling, projected.tagged, expected)
        # These familiar informal spellings are retained as exceptions, and
        # their explicit aa alternatives share the Persian word analysis.
        for single, double, script in (
                ('salam', 'salaam', 'سلام'), ('iran', 'iraan', 'ایران'),
                ('farsi', 'faarsi', 'فارسی'), ('ketab', 'ketaab', 'کتاب'),
                ('khane', 'khaane', 'خانه'), ('farda', 'fardaa', 'فردا')):
            expected = persian(script)
            for spelling in (single, double):
                assert word(spelling) == expected, (spelling, word(spelling), expected)
                assert project((spelling,), frontend, allow_fallback=False,
                               pinglish=True).tagged == expected, spelling
        assert word('ketabha') == word('ketabhaa') == persian('کتاب‌ها')
        assert word('khaanandeha') == word('khaanandehaa') == persian('خواننده‌ها')
        # A suffix must not reinterpret the preceding single a in a stem.
        assert word('baraan') == '`[.0bA.1ran]'
        assert project(('baraan',), frontend, allow_fallback=False,
                       pinglish=True).tagged == '`[.0bA.1ran]'
    finally:
        frontend.close()

    def render(language, text):
        owner = make_language_engine(args.library, args.library, None,
                                     Path('/unavailable-espeak'), Path('/unavailable'),
                                     language=language)
        try:
            owner.set_parameter(SAMPLE_RATE, 3)
            owner.set_parameter(NATIVE_SPEED, 82)
            audio = bytearray()
            assert owner.speak((text,), audio.extend, lambda _: None) and audio, text
            return bytes(audio)
        finally:
            owner.close()

    pinglish = PINGLISH_PROBE.id
    assert render(pinglish, 'bar baaz') != render(pinglish, 'baar baaz')
    for roman, script in (('bar', 'بر'), ('baar', 'بار'),
                          ('salam', 'سلام'), ('salaam', 'سلام'),
                          ('iran', 'ایران'), ('iraan', 'ایران')):
        assert render(pinglish, roman) == render(PERSIAN_PROBE.id, script), (roman, script)
    print(f'Passed {len(pairs)} a/aa quality pairs, common exceptions and shared audio paths.')


if __name__ == '__main__':
    main()
