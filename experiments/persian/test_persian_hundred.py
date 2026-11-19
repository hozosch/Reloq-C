"""Check the short vowel of Persian صد in cardinal number groups."""
from __future__ import annotations

import argparse
import ctypes
import json
from pathlib import Path
import re

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    translate = dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    translate.restype = ctypes.c_int
    dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)

    def project(text):
        raw = text.encode('utf8')
        output, mapping, error = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        status = translate(raw, len(raw), ctypes.byref(output), ctypes.byref(mapping), ctypes.byref(error))
        assert status == 0 and output.value, (text, status, error.value)
        try:
            tag = ctypes.string_at(output).decode('ascii')
            offsets = [mapping[i] for i in range(len(raw) + 1)]
            assert offsets == sorted(offsets) and offsets[-1] == len(tag), text
            return tag
        finally:
            dll.eciPersianRelease(output)
            dll.eciPersianRelease(mapping)

    source = Path(__file__).with_name('direct_fa_numbers.inc').read_text(encoding='utf8')
    body = source.split('fa_number_groups[] = {', 1)[1].split('};', 1)[0]
    groups = [json.loads(line.strip().rstrip(',')) for line in body.splitlines()
              if line.strip().startswith('"')]
    assert len(groups) == 1000
    fa = str.maketrans('0123456789', '۰۱۲۳۴۵۶۷۸۹')
    ar = str.maketrans('0123456789', '٠١٢٣٤٥٦٧٨٩')
    phones = lambda value: re.sub(r'\.[012]', '.#', value)
    for number in range(100, 200):
        old = groups[number]
        assert "s'A:'" in old, number
        expected = old.replace("s'A:'", 'sA', 1)
        plain = str(number)
        for text in (plain, plain.translate(fa), plain.translate(ar)):
            actual = project(text)
            assert phones(actual) == phones(expected), (text, actual, expected)
            assert "s'A:'" not in actual, (text, actual)
    assert project('100') == project('صد') == '`[.1sAd]'
    for text in ('101', '123', '167', '۱۹۹', '۱۲۰', '۱۷۱', '-123',
                 '123.5', '123000', 'عدد ۱۲۳ است.'):
        tag = project(text)
        assert re.search(r'\.[012]sA', tag) and "s'A:'" not in tag, (text, tag)

    # Exact-size output buffers must account for the shortened representation.
    number = dll.eciPersianNumber
    number.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
                       ctypes.c_void_p, ctypes.c_size_t)
    number.restype = ctypes.c_int
    wanted = project('100').encode('ascii')
    output, used = ctypes.create_string_buffer(len(wanted) + 1), ctypes.c_size_t()
    assert number(b'100', 3, ctypes.byref(used), output, len(output)) == 0
    assert used.value == 3 and output.value == wanted
    small = ctypes.create_string_buffer(b'keep', len(wanted))
    assert number(b'100', 3, ctypes.byref(used), small, len(wanted)) == -2

    # The actual first syllable, not only the phoneme spelling, gets shorter.
    engine, scope = voice(args.library, 0x90000)
    comparisons = 0
    try:
        for gender in (0, 1):
            for speed in (40, 65, 100):
                for parameter, value in ((GENDER, gender), (NATIVE_SPEED, speed), (SAMPLE_RATE, 3)):
                    engine.set_parameter(parameter, value)
                for text in ('100', '101', '123', '167', '199', '120', '171'):
                    short = project(text)
                    long = short.replace('sA', "s'A:'", 1)
                    assert long != short
                    current, _ = capture(engine, scope, short, True)
                    previous, _ = capture(engine, scope, long, True)
                    c0, c1 = current.syllables[0]
                    p0, p1 = previous.syllables[0]
                    if short.startswith('`[.0sA'):
                        # An unstressed initial long marker is now planned
                        # with the same duration as the short phone. Both
                        # tags must therefore produce exactly the same span.
                        assert c1 - c0 == p1 - p0, (gender, speed, text, current.syllables, previous.syllables)
                    else:
                        assert c1 - c0 < p1 - p0, (gender, speed, text, current.syllables, previous.syllables)
                    comparisons += 1
    finally:
        engine.close()
    print(f'Passed 300 hundred-group spellings, source maps and {comparisons} audible timing comparisons.')


if __name__ == '__main__':
    main()
