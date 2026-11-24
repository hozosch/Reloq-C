"""Check recovered Persian words and preserve previously native analyses."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from compile_direct_classified_words import inventory
from hybrid_fa_frontend import DirectFirstPersian


def phones(tag):
    # The native Persian duration rule deliberately removes eSpeak's vowel
    # length markup while retaining vowel identity.
    tag = re.sub(r"'([Aaeiou]):'", r'\1', tag)
    return re.sub(r'\.[012]|[`\[\]~ ]', '', tag)


# Readings that were changed on purpose after review. Everything else that
# an earlier core analyzed must stay identical; these must match the new
# reading instead, so that neither an accidental change nor a silent return
# to the old reading passes.
ZWNJ = '\u200c'
REVIEWED_CHANGES = {
    'نخست': '`[.0no.1Gost]',
    f'گفت{ZWNJ}وگو': '`[.0gof.0to.1gu]',
    f'آب{ZWNJ}وهوا': '`[.0a.0bo.0hA.1va]',
    f'حمل{ZWNJ}ونقل': '`[.0hAm.0lo.1nARl]',
    f'فوق{ZWNJ}العاده': '`[.0fo.0Ro.0la.1de]',
    f'خارق{ZWNJ}العاده': '`[.0Ga.0re.0Ro.0la.1de]',
    'دارالفنون': '`[.0da.0rol.0fo.1nun]',
    f'دایم{ZWNJ}الخمر': '`[.0da.0je.0mol.1GAmr]',
    f'ضرب{ZWNJ}الاجل': '`[.0zAr.0bo.0lA.1JAl]',
    f'آیت{ZWNJ}الله': '`[.0a.0jA.0tol.1lahw]',
    f'امین{ZWNJ}الله': '`[.0A.0mi.0nol.1lahw]',
    f'بین{ZWNJ}الله': '`[.0bej.0nol.1lahw]',
    f'حزب{ZWNJ}الله': '`[.0hez.0bol.1lahw]',
    f'یوم{ZWNJ}الله': '`[.0jo.0mol.1lahw]',
    f'بسم{ZWNJ}الله': '`[.0bes.0mel.1lahw]',
    'انشاءالله': '`[.0en.0Sa.0wAl.1lahw]',
    'انشاالله': '`[.0en.0Sa.0wAl.1lahw]',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    examples = {
        'سرمربی': '`[.0sAr.0mo.1rAb.0bi]',
        'سرمربیان': '`[.0sAr.2mo.0rAb.1bi.0jan]',
        'پرسابقه': "`[.0por.1sa.0be.2R'e:']",
        'رحمتی': '`[.0rAh.1mA.0ti]',
        'باقری': '`[.1ba.0Re.2ri]',
        'پزشکیان': '`[.0pe.0zeS.0ki.1an]',
        'بیرانوند': '`[.0bej.0ran.1vAnd]',
    }
    with DirectFirstPersian(args.library, None, None, None) as current, \
         DirectFirstPersian(args.previous, None, None, None) as old:
        for word, reference in examples.items():
            analysis = current.analyze_direct(word)
            assert analysis and len(analysis.parts) == 1, (word, analysis)
            assert phones(analysis.parts[0].text) == phones(reference), (word, analysis, reference)
            for phrase in (f'{word} آمد.', f'من {word} آمد.'):
                result = current.analyze_direct(phrase)
                assert result and phones(reference) in phones(result.parts[0].text), (word, phrase, result)
        # A whole-word recovery must never swallow the established
        # half-space/prefix fallback route.
        for word in ('نمی‌آراست',):
            assert old.analyze_direct(word) is current.analyze_direct(word) is None
            assert old.project_fallback_word(word) == current.project_fallback_word(word), word
        checked = 0
        reviewed = set()
        for word in inventory():
            previous = old.analyze_direct(word)
            if previous is None:
                continue
            current_result = current.analyze_direct(word)
            if word in REVIEWED_CHANGES:
                assert (current_result and len(current_result.parts) == 1 and
                        current_result.parts[0].text == REVIEWED_CHANGES[word]), (word, current_result)
                reviewed.add(word)
            else:
                assert current_result == previous, (word, previous, current_result)
            checked += 1
        assert checked > 30000, checked
        assert reviewed == set(REVIEWED_CHANGES), set(REVIEWED_CHANGES) - reviewed
        # Explicit vowels in the established Roman-script lexicon must not
        # change merely because a Persian-script reading was recovered.
        table = (ROOT / 'experiments/persian/direct_fa_pinglish_lexicon.inc').read_text(encoding='utf8')
        roman_entries = re.findall(r'^\s*\{"([a-z]+)", "([^"\\]+)"\},', table, re.M)
        roman_words = [roman for roman, _ in roman_entries]
        # A Roman spelling follows the reading of its Persian word, so the
        # reviewed changes above reach it too; it must still be analyzed.
        roman_reviewed = {roman for roman, script in roman_entries if script in REVIEWED_CHANGES}
        # These newly admitted symbol-name bases were already pronounced
        # correctly by Pinglish; recovery must not need a second alias.
        roman_words.extend(('hejaai', 'kaarun'))
        for frontend in (old, current):
            fn = frontend._dll.eciPersianPinglishWord
            fn.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
            fn.restype = ctypes.c_int
        roman_checked = 0
        for word in roman_words:
            raw = word.encode('ascii'); outputs = []
            for frontend in (old, current):
                buf = ctypes.create_string_buffer(4096)
                status = frontend._dll.eciPersianPinglishWord(raw, len(raw), buf, len(buf))
                outputs.append((status, buf.value))
            if outputs[0][0] == 1:
                if word in roman_reviewed:
                    assert outputs[1][0] == 1, (word, outputs)
                else:
                    assert outputs[0] == outputs[1], (word, outputs)
                roman_checked += 1
        assert roman_checked > 30000, roman_checked
    print(f'Passed 7 reported words and {checked} Persian plus {roman_checked} Pinglish existing analyses.')


if __name__ == '__main__':
    main()
