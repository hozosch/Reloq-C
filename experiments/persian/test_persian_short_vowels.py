"""Check automatic Persian short vowels and U before coronal consonants."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from direct_fa_sequence import project
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine
from test_fa_cluster_final_timing import ms, projector
from test_persian_r_after_i_contexts import at_fraction, capture
from test_us_articulation import voice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    dll = current._dll
    convert = projector(dll)
    roman = dll.eciPersianPinglishWord
    roman.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    roman.restype = ctypes.c_int
    stress = dll.eciPersianStressUtf8
    stress.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    stress.restype = ctypes.c_int
    character = dll.eciPersianCharacterResolve
    character.argtypes = (ctypes.c_uint32, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_int))
    character.restype = ctypes.c_int
    number = dll.eciPersianNumber
    number.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
                       ctypes.c_void_p, ctypes.c_size_t)
    number.restype = ctypes.c_int

    pairs = (('خصوصیات', 'khosusiaat'), ('خصوصی', 'khosusi'),
             ('عمومی', 'omumi'), ('ایران', 'iraan'))
    for written, latin in pairs:
        expected = convert(written)
        raw = latin.encode('ascii'); result = ctypes.create_string_buffer(4096)
        assert roman(raw, len(raw), result, len(result)) == 1
        assert result.value.decode('ascii') == expected, (written, latin, expected, result.value)
        assert not re.search(r"'[Aaeiou]:'", expected), (written, expected)
    assert convert('خصوصیات') == '`[.0Go.0su.0si.1at]'
    assert convert('ایران') == '`[.0i.1ran]'
    for word, source in (('عمومی', "`[.0o.0m'u:'.1m'i:']"),
                         ('سوئد', "`[.0s'u:'.1w'e:'d]")):
        raw = word.encode('utf8'); output = ctypes.create_string_buffer(source.encode('ascii'), 512)
        assert stress(raw, len(raw), output, len(output)) == 0
        assert not re.search(rb"'[Aaeiou]:'", output.value), (word, output.value)
    for cp in map(ord, ('ب', 'ر', '؟', '@', '🙂')):
        output = ctypes.create_string_buffer(2048); kind = ctypes.c_int()
        assert character(cp, output, len(output), ctypes.byref(kind)) == 1
        assert kind.value == 0 and not re.search(rb"'[Aaeiou]:'", output.value), (cp, output.value)
    for value in ('2', '7', '8', '400', '401', '1000000'):
        raw = value.encode('ascii'); output = ctypes.create_string_buffer(4096); consumed = ctypes.c_size_t()
        assert number(raw, len(raw), ctypes.byref(consumed), output, len(output)) == 0
        assert consumed.value == len(raw) and not re.search(rb"'[Aaeiou]:'", output.value)
        # Exact output capacity must work after the automatic markers vanish.
        exact = ctypes.create_string_buffer(len(output.value) + 1)
        assert number(raw, len(raw), ctypes.byref(consumed), exact, len(exact)) == 0
        assert exact.value == output.value, value

    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                for speed in (40, 82, 100):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    neutral = capture(current, scope, '`[.1su]')
                    u_target = [at_fraction(neutral, 1, .5, col) for col in (9, 13)]
                    for consonant in 'sztd':
                        tag = f'`[.0Go.0su.0{consonant}i.1at]'
                        sample = capture(current, scope, tag)
                        u = [at_fraction(sample, 3, .5, col) for col in (9, 13)]
                        assert all(abs(a-b) <= 80 for a, b in zip(u, u_target)), (
                            consonant, rate, gender, head, speed, u, u_target)
                        assert ms(sample, 3) >= 10, (consonant, rate, gender, head, speed)
                        if old:
                            before = capture(old, old_scope, tag)
                            assert ms(sample, 3) < ms(before, 3), (tag, rate, gender, speed)
                            assert capture(current, scope, tag, False).pcm == \
                                   capture(old, old_scope, tag, False).pcm, ('Finnish changed', tag)
                        checks += 1
                    if old:
                        # Explicit input outside the changed U/consonant context
                        # continues to distinguish a long from a short vowel.
                        explicit = "`[.0o.0m'u:'.1mi]"
                        assert capture(current, scope, explicit).pcm == capture(old, old_scope, explicit).pcm
    finally:
        current.close()
        if old: old.close()

    for written, latin in pairs[:3]:
        pcm = []
        for language, text in ((PERSIAN_PROBE.id, written), (PINGLISH_PROBE.id, latin)):
            owner = make_language_engine(args.library, args.library, None,
                                         Path('/unavailable-espeak'), Path('/unavailable'), language=language)
            try:
                owner.set_parameter(SAMPLE_RATE, 3); owner.set_parameter(NATIVE_SPEED, 82)
                audio = bytearray()
                assert owner.speak((text,), audio.extend, lambda _: None) and audio
                frontend = owner._activate(owner._key(language, '1'))._frontend
                assert project((text,), frontend, allow_fallback=False,
                               pinglish=language == PINGLISH_PROBE.id).tagged == convert(written)
                pcm.append(bytes(audio))
            finally: owner.close()
        assert pcm[0] == pcm[1], (written, latin)
    print(f'Passed {checks} U contexts, automatic short vowels, numbers, symbols and script modes.')


if __name__ == '__main__':
    main()
