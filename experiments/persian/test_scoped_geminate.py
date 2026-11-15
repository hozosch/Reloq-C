"""Verify selected Persian affricates against the accepted listening probe."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re

from direct_fa_sequence import (
    EciIndex, EciText, ScopedDirectPersianEngine, _mark_geminated_affricates,
)
from hybrid_fa_frontend import DirectFirstPersian
from listening import render
from test_selected_probe import engine, set_confirmed_persian_probe
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def mark(parts):
    return _mark_geminated_affricates(tuple(parts))


def marker_tests():
    parts = (EciIndex(7), EciText('`[.1AJ.0Job] '), EciIndex(8),
             EciText('`[.1AJ.0AJ.1Job]'))
    output, selected = mark(parts)
    assert selected == ((1 << 16) | 1, (2 << 16) | 2)
    assert [p.value for p in output if isinstance(p, EciIndex)] == [7, 8]
    assert [p.text for p in output if isinstance(p, EciText)] == [
        '`[.1A.0Job] ', '`[.1AJ.0A.1Job]']
    unchanged, selected = mark((EciText('`[.1AdZ.0JJob]'),))
    assert unchanged[0].text == '`[.1AdZ.0JJob]' and not selected
    unchanged, selected = mark((EciText('`[.1AJ.0A]'),))
    assert unchanged[0].text == '`[.1AJ.0A]' and not selected


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--native-library', required=True, type=Path)
    args = parser.parse_args()
    marker_tests()
    unused = Path('/nonexistent/reloq-reference')
    with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend:
        voice = ScopedDirectPersianEngine(args.probe, frontend)
        reference = engine(args.reference, 0x90000)
        try:
            def say(word):
                pcm, indexes = bytearray(), []
                assert voice.speak((EciIndex(1), word, EciIndex(2)), pcm.extend, indexes.append)
                assert indexes == [1, 2] and pcm
                return bytes(pcm)

            def compare(word):
                original = frontend.analyze_direct(word).parts[0].text
                pieces, selected = mark((EciText(original),))
                assert len(selected) == 1 and pieces[0].text.count('J') == 1, (word, original)
                set_confirmed_persian_probe(d=2, z=2, gem=1)
                expected = render(reference, pieces[0].text)
                actual = say(word)
                assert actual == expected, (word, original, len(actual), len(expected))

            lexicon = Path(__file__).with_name('direct_fa_lexicon.inc').read_text(encoding='utf8')
            entries = re.findall(r'\{"([^"\\]+)",\s*"(`\[[^"\]]+\])"', lexicon)
            words = [word for word, tag in entries
                     if re.search(r'J(?:\.[012])?J', tag) and tag.count('J') == 2]
            assert len(words) >= 40
            for word in words:
                compare(word)
            for phrase in ('سلام تعجب توجه', 'تعجب، توجه؟'):
                analysis = frontend.analyze_direct(phrase)
                assert analysis and len(analysis.parts) == 1, phrase
                pieces, selected = mark((EciText(analysis.parts[0].text),))
                assert len(selected) == 2, (phrase, selected)
                set_confirmed_persian_probe(d=2, z=2, gem=1)
                assert say(phrase) == render(reference, pieces[0].text), phrase
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for target in (voice, reference):
                        target.set_parameter(SAMPLE_RATE, rate)
                        target.set_parameter(GENDER, gender)
                    for word in ('تعجب', 'توجه'):
                        compare(word)
                    for word in ('جادو', 'سلام'):
                        analysis = frontend.analyze_direct(word)
                        assert analysis and analysis.parts
                        set_confirmed_persian_probe(d=2, z=2, gem=0)
                        assert say(word) == render(reference, analysis.parts[0].text), word
            for speed in (25, 75):
                for target in (voice, reference):
                    target.set_parameter(NATIVE_SPEED, speed)
                compare('تعجب')

            # A native word can also contain a geminate and a separate J.
            # Selecting ordinal one versus ordinal two must affect the
            # corresponding earlier versus later part of the waveform.
            original = frontend.analyze_direct('حجاج').parts[0].text
            parts, selected = mark((EciText(original),))
            assert selected == ((1 << 16) | 1,) and parts[0].text.count('J') == 2
            tag = parts[0].text

            def raw(pairs):
                previous = voice._scope(1)
                assert previous == 0
                data = (ctypes.c_uint32 * len(pairs))(*pairs) if pairs else None
                try:
                    assert voice._geminates(data, len(pairs)) == 0
                    return render(voice._voice, tag)
                finally:
                    assert voice._geminates(None, 0) == 0
                    assert voice._scope(previous) == 1

            plain = raw(())
            first, second = raw(((1 << 16) | 1,)), raw(((1 << 16) | 2,))
            assert len(first) > len(plain) and len(second) > len(plain)
            first_difference = next(i for i, (a, b) in enumerate(zip(plain, first)) if a != b)
            second_difference = next(i for i, (a, b) in enumerate(zip(plain, second)) if a != b)
            assert first_difference < second_difference, (first_difference, second_difference)
            assert say('حجاج') == first
        finally:
            voice.close()
            reference.close()
    print(f'Passed {len(words)} direct geminate words, separate J ordinals, indexes and voice settings.')


if __name__ == '__main__':
    main()
