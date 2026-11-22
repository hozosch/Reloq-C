"""Verify the native Pinglish map against the shared Persian word pipeline."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random
import re


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    native = getattr(dll, 'eciPersianPinglishWord', None) or dll.reloq_fa_pinglish_word
    native.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    native.restype = ctypes.c_int
    translate = dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    translate.restype = ctypes.c_int
    dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)
    complete = dll.eciPersianProjectTextCompleteUtf8
    complete.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                         ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t,
                         ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t),
                         ctypes.POINTER(ctypes.c_size_t))
    complete.restype = ctypes.c_int

    def pinglish(word):
        source = word.encode('utf8')
        output = ctypes.create_string_buffer(4096)
        status = native(source, len(source), output, len(output))
        return status, output.value.decode('ascii')

    def persian(word):
        source = word.encode('utf8')
        output, mapping, error = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        status = translate(source, len(source), ctypes.byref(output), ctypes.byref(mapping), ctypes.byref(error))
        assert status == 0, (word, status, error.value)
        try:
            return ctypes.string_at(output).decode('ascii')
        finally:
            dll.eciPersianRelease(output)
            dll.eciPersianRelease(mapping)

    def complete_phrase(text):
        source = text.encode('utf8')
        output, error = ctypes.c_void_p(), ctypes.c_size_t()
        offsets = (ctypes.c_size_t * 2)(0, len(source))
        is_index = (ctypes.c_ubyte * 2)(1, 1)
        targets = (ctypes.c_size_t * 2)()
        status = complete(source, len(source), 1, offsets, is_index, 2,
                          ctypes.byref(output), targets, ctypes.byref(error))
        assert status == 0, (text, status, error.value)
        try:
            tag = ctypes.string_at(output).decode('ascii')
            assert targets[0] <= targets[1] <= len(tag), (text, tuple(targets), tag)
            return tag
        finally:
            dll.eciPersianRelease(output)

    def check_a_quality(roman, tag):
        expected = ''.join('a' if match.group() == 'aa' else 'A'
                           for match in re.finditer(r'aa|a', roman.lower()))
        actual = ''.join(phone for phone in tag if phone in 'aA')
        assert actual == expected, (roman, tag, expected, actual)

    table = Path(__file__).with_name('direct_fa_pinglish_lexicon.inc').read_text(encoding='utf8')
    entries = re.findall(r'^\s*\{"([a-z]+)", "([^"\\]+)"\},', table, re.M)
    assert len(entries) > 25000
    checked = 0
    for roman, script in entries:
        if 'ph' in roman or 'th' in roman or 'ooo' in roman:
            continue  # Existing explicit English/fallback routing.
        result = pinglish(roman)
        assert result == (1, persian(script)), (roman, script, result, persian(script))
        check_a_quality(roman, result[1])
        checked += 1
    # Exercise unresolved and inflected words too: no stored spelling or
    # productive ending is allowed to change an individual a into aa.
    rng = random.Random(20261121)
    parts = ('b', 'd', 'f', 'g', 'h', 'k', 'l', 'm', 'n', 'r', 's', 't',
             'sh', 'kh', 'ch', 'gh', 'z', 'a', 'aa', 'e', 'i', 'o', 'u')
    for _ in range(3000):
        roman = ''.join(rng.choice(parts) for _ in range(rng.randrange(2, 8)))
        status, result = pinglish(roman)
        if status == 1:
            check_a_quality(roman, result)
    for roman, script in (('khaane', 'خانه'), ('khaaneh', 'خانه'), ('chetori', 'چطوری'),
                          ('paraantez', 'پرانتز'), ('raayaane', 'رایانه'),
                          ('mikhaaham', 'می‌خواهم'), ('miravam', 'می‌روم'),
                          ('nemiravam', 'نمی‌روم'), ('tehraani', 'تهرانی'),
                          ('futbaal', 'فوتبال'), ('footbaal', 'فوتبال'),
                          ('ketaabam', 'کتابم'), ('ketaabhaa', 'کتاب‌ها'),
                          ('khubtar', 'خوبتر'), ('donyaaye', 'دنیای'), ('majaazi', 'مجازی')):
        assert pinglish(roman) == (1, persian(script)), (roman, script, pinglish(roman), persian(script))
        assert pinglish(roman.upper()) == pinglish(roman), roman
    for word in ('phone', 'this', 'zooon'):
        assert pinglish(word)[0] == 0, word
    # Only the complete, reference-free path tolerates elongated informal
    # vowel spellings. It retains short versus doubled vowels and lets weak
    # following words keep their normal context.
    for extended, canonical in (('zooon ast', 'zoon ast'), ('zooooon raa', 'zoon raa'),
                                ('salaaaam', 'salaam'), ('salaaaaam', 'salaam'),
                                ('zooon', 'zoon')):
        assert complete_phrase(extended) == complete_phrase(canonical), extended
    assert '`00 `[.0Ast]' in complete_phrase('zooon ast')
    assert '`[.0ra]' in complete_phrase('zooooon raa')
    # Unknown spellings are represented with Persian letters and explicit
    # vowel marks before the shared Persian text parser is called.
    for roman, script in (('sevinaa', 'سِوینا'), ('zarakhi', 'زَرَخِی'),
                          ('parlap', 'پَرْلَپْ'), ('mergish', 'مِرْگِیشْ')):
        assert pinglish(roman) == (1, persian(script)), (roman, script, pinglish(roman), persian(script))
    assert persian('سوینا') == persian('سِوینا')
    extra = Path(__file__).with_name('direct_fa_pinglish_extra.inc').read_text(encoding='utf8')
    more = re.findall(r'^\s*\{"([a-z]+)", "([^"\\]+)"\},', extra, re.M)
    assert len(more) >= 4
    for roman, script in more:
        assert pinglish(roman) == (1, persian(script)), (roman, script)
        check_a_quality(roman, pinglish(roman)[1])
    aliases = Path(__file__).with_name('direct_fa_pinglish_aliases.inc').read_text(encoding='utf8')
    for roman in re.findall(r'\{"([a-z]+)","', aliases):
        if len(roman) == 1:
            continue  # Single letters are handled by the phrase/character path.
        status, tag = pinglish(roman)
        assert status == 1, roman
        check_a_quality(roman, tag)
    assert pinglish('zirkhat') == (1, persian('زیرخط'))
    print(f'Passed {checked} native Pinglish/Persian word comparisons and morphology checks.')


if __name__ == '__main__':
    main()
