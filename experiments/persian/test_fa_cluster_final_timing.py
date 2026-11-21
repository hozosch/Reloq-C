"""Check native short cluster vowels and phrase-final Persian i/o."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import at_fraction, capture
from test_us_articulation import voice


def projector(dll):
    translate = dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t))
    dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)
    def project(text):
        raw = text.encode('utf8')
        output, mapping, error = ctypes.c_void_p(), ctypes.c_void_p(), ctypes.c_size_t()
        status = translate(raw, len(raw), ctypes.byref(output), ctypes.byref(mapping), ctypes.byref(error))
        assert status == 0, (text, status, error.value)
        try:
            return ctypes.string_at(output).decode('ascii')
        finally:
            dll.eciPersianRelease(output)
            dll.eciPersianRelease(mapping)
    return project


def ms(result, index):
    begin, end = result.syllables[index]
    return (end - begin) / 10000


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    project = projector(current._dll)
    ping = current._dll.eciPersianPinglishWord
    ping.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    ping.restype = ctypes.c_int
    examples = {
        'عکس': '`[.1Aks]',
        'عکسی': '`[.1Ak.0si]',
        'ابر': '`[.1Abr]',
        'اکثر': '`[.0Ak.1sAr]',
        'برعکس': '`[.0bA.1rAks]',
    }
    for word, expected in examples.items():
        assert project(word) == expected, (word, project(word), expected)
    for roman, written in (('aks', 'عکس'), ('aksi', 'عکسی'), ('abr', 'ابر'),
                           ('aksar', 'اکثر'), ('baraks', 'برعکس')):
        encoded = roman.encode('ascii')
        output = ctypes.create_string_buffer(2048)
        assert ping(encoded, len(encoded), output, len(output)) == 1, roman
        assert output.value.decode('ascii') == project(written), (roman, output.value, project(written))
    assert project('2') == project('دو') == '`[.1do]'
    # Generated words use the short variant of each vowel, without changing
    # the independent A/a timbres. Explicit length remains available below.
    assert project('ایران') == '`[.0i.1ran]'
    assert "'u:'" not in project('خوب')
    assert project('اساسی').startswith('`[.0A')

    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    # The short nucleus is the normal short phoneme, while an
                    # explicitly supplied long phonetic input stays available.
                    short = capture(current, scope, project('عکس'))
                    explicit = capture(current, scope, "`[.1'A:'ks]")
                    assert ms(short, 0) < ms(explicit, 0), (rate, gender, speed)
                    for column in (9, 13, 15, 17):
                        assert abs(at_fraction(short, 0, .5, column) -
                                   at_fraction(explicit, 0, .5, column)) <= 50, (column, rate, gender, speed)
                    for tag, middle, index in (
                        ('`[.1Ak.0si]', '`[.1Ak.0si] `[.1hAft]', 3),
                        ('`[.1do]', '`[.1do] `[.1hAft]', 1),
                        ('`[.1si]', '`[.1si] `[.1hAft]', 1),
                    ):
                        final = capture(current, scope, tag)
                        medial = capture(current, scope, middle)
                        assert ms(final, index) > ms(medial, index), (tag, rate, gender, speed)
                        if old:
                            prior = capture(old, old_scope, tag)
                            before_medial = capture(old, old_scope, middle)
                            delta = ms(final, index) - ms(prior, index)
                            assert 0 < delta <= 35, (tag, rate, gender, speed, delta)
                            if tag == '`[.1Ak.0si]':
                                medial_delta = ms(medial, index) - ms(before_medial, index)
                                assert 0 < medial_delta <= 15, (tag, rate, gender, speed, medial_delta)
                            else:
                                assert medial.pcm == before_medial.pcm, (tag, rate, gender, speed, 'medial changed')
                        checks += 1
                    if old:
                        # An isolated short-vowel mark, consonant-final word,
                        # and explicitly supplied long A keep their sound.
                        for tag in ('`[.1o]', '`[.1e]', '`[.1hAft]', "`[.1'A:'ks]"):
                            now = capture(current, scope, tag)
                            before = capture(old, old_scope, tag)
                            assert now.pcm == before.pcm, (tag, rate, gender, speed)
                        assert capture(current, scope, '`[.1si]', False).pcm == \
                               capture(old, old_scope, '`[.1si]', False).pcm, ('Finnish changed', rate, gender, speed)
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {checks} Persian/Pinglish cluster and final-vowel context checks.')


if __name__ == '__main__':
    main()
