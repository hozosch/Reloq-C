"""Check the reported weak initial vowel and short standalone marks."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import at_fraction, capture
from test_us_articulation import voice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    engine, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)

    def project(voice_engine, text):
        dll = voice_engine._dll; fun = dll.eciPersianTranslateUtf8
        fun.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                        ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t))
        dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)
        raw = text.encode('utf8'); result = ctypes.c_void_p(); mapping = ctypes.c_void_p(); error = ctypes.c_size_t()
        assert fun(raw, len(raw), ctypes.byref(result), ctypes.byref(mapping), ctypes.byref(error)) == 0
        try:
            return ctypes.string_at(result).decode('ascii')
        finally:
            dll.eciPersianRelease(result); dll.eciPersianRelease(mapping)

    current_tag = project(engine, 'اساسی')
    assert current_tag.startswith("`[.0'A:'"), current_tag
    short_tag = current_tag.replace("'A:'", 'A', 1)
    old_tag = project(previous, 'اساسی') if previous else None
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 20)):
                for speed in (40, 82):
                    for item in (engine, previous):
                        if item:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed)):
                                item.set_parameter(key, value)
                    result = capture(engine, scope, current_tag)
                    short = capture(engine, scope, short_tag)
                    first = lambda item: item.syllables[0][1] - item.syllables[0][0]
                    assert first(result) == first(short), (rate, gender, speed, current_tag)
                    if previous:
                        before = capture(previous, old_scope, old_tag)
                        assert first(result) < first(before), (rate, gender, speed, first(result), first(before))
                    for mark, vowel in (('َ', 'A'), ('ُ', 'o'), ('ِ', 'e')):
                        tag = project(engine, mark)
                        assert tag == '`[.1' + vowel + ']'
                        current = capture(engine, scope, tag)
                        reference = capture(engine, scope, "`[.1'" + vowel + ":']")
                        assert first(current) < first(reference), (mark, rate, gender, speed)
                        if previous:
                            before_short = capture(previous, old_scope, tag)
                            before_long = capture(previous, old_scope, "`[.1'" + vowel + ":']")
                            assert reference.pcm == before_long.pcm, (mark, rate, gender, speed, 'long vowel changed')
                            if vowel == 'e':
                                assert first(current) * 100 < first(before_short) * 80, (
                                    mark, rate, gender, speed, 'isolated short vowel not shortened')
                                native = capture(engine, scope, tag, selected=False)
                                old_native = capture(previous, old_scope, tag, selected=False)
                                assert native.pcm == old_native.pcm, (mark, rate, gender, speed, 'Finnish changed')
                            else:
                                assert current.pcm == before_short.pcm, (mark, rate, gender, speed, 'other mark changed')
                    if previous:
                        # Short E with a consonant is a normal word and must
                        # retain the listener-approved timing and sound.
                        tag = '`[.1be]'
                        assert capture(engine, scope, tag).pcm == capture(previous, old_scope, tag).pcm, (
                            tag, rate, gender, speed, 'short vowel in word changed')
                    for word in ('سوئد', 'سوئدی', 'ساعت'):
                        # Preserve the previously accepted glottal closure
                        # and its gradual voice onset for both hamza and ayn.
                        tag = project(engine, word)
                        pronunciation = capture(engine, scope, tag)
                        a, b = (x / 10000.0 for x in pronunciation.syllables[2])
                        rows = pronunciation.frames()
                        closure = [row.controls[2] for row in rows if a < row.tick <= b]
                        after = next((row.controls[2] for row in rows if row.tick > b and row.controls[2] > 0), 0)
                        assert 0 < b-a <= 10 and closure and min(closure) == 0 and 0 < after < 10, (
                            word, rate, gender, speed, 'glottal closure or voice onset changed')
                        if word != 'ساعت':
                            # The E target used to pull the preceding U all
                            # the way up during a short, voiced transition.
                            # Its glottal midpoint should instead remain
                            # between the U and E nuclei.
                            u, edge, e = (at_fraction(pronunciation, i, .5, 13) for i in (1, 2, 3))
                            assert u < edge < e and 4 * (edge-u) < 3 * (e-u), (
                                word, rate, gender, speed, 'sharp E transition', u, edge, e)
                        if previous:
                            before = capture(previous, old_scope, tag)
                            assert pronunciation.syllables == before.syllables, (word, rate, gender, speed, 'timing changed')
                            old_rows = before.frames()
                            assert len(rows) == len(old_rows)
                            for new, old in zip(rows, old_rows):
                                assert new.tick == old.tick and all(
                                    new.controls[i] == old.controls[i] for i in (0, 1, 2, 7, 8)), (
                                        word, rate, gender, speed, 'glottal source changed')
                            if word == 'ساعت':
                                assert pronunciation.pcm == before.pcm, (word, rate, gender, speed, 'ayn changed')
                            else:
                                assert pronunciation.pcm != before.pcm, (word, rate, gender, speed, 'E edge unchanged')
                                for segment in (1, 3):
                                    for column in (9, 13, 15, 17):
                                        assert abs(at_fraction(pronunciation, segment, .5, column) -
                                                   at_fraction(before, segment, .5, column)) < .01, (
                                            word, rate, gender, speed, 'vowel nucleus changed', segment, column)
                    if previous:
                        # Explicit initial closure and hamza after a
                        # consonant keep their existing glottal attack.
                        for tag in ('`[.1wa]', '`[.1we]', "`[.0s'u:'.1wad]",
                                    "`[.1'u:'] `[.1we]", "`[.0mAs.1w'u:'l]"):
                            assert capture(engine, scope, tag).pcm == capture(previous, old_scope, tag).pcm, (
                                tag, rate, gender, speed, 'unrelated glottal changed')
                    count += 1
    finally:
        engine.close()
        if previous:
            previous.close()
    print(f'Passed {count} vowel-duration, restored glottal and contextual E comparisons.')


if __name__ == '__main__':
    main()
