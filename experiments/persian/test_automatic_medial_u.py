"""Check native timing for automatically marked weak medial Persian U."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
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
    engine, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    convert = projector(engine._dll)
    ping = engine._dll.eciPersianPinglishWord
    ping.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    ping.restype = ctypes.c_int
    for word, expected in (('عمومی', '`[.0o.0mu.1mi]'), ('خصوصی', '`[.0Go.0su.1si]'),
                           ('ضروری', '`[.0zA.0ru.1ri]'), ('معمولی', '`[.0mAw.0mu.1li]'),
                           ('حقوقی', '`[.0ho.0Ru.1Ri]')):
        assert convert(word) == expected, (word, convert(word))
    for roman, word in (('omumi', 'عمومی'), ('khosusi', 'خصوصی'), ('zaruri', 'ضروری')):
        b = roman.encode('ascii'); output = ctypes.create_string_buffer(1024)
        assert ping(b, len(b), output, len(output)) == 1
        assert output.value.decode('ascii') == convert(word), (roman, output.value)

    # The eSpeak/reference path sends its already-generated group through
    # the same native policy. Neither the vowel identity nor stress changes.
    stress = engine._dll.eciPersianStressUtf8
    stress.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    stress.restype = ctypes.c_int
    for word in ('عمومی', 'omumi', 'ناشناخته'):
        source = word.encode('utf8'); tag = ctypes.create_string_buffer(b"`[.0o.0m'u:'.1mi]")
        assert stress(source, len(source), tag, len(tag)) == 0
        assert tag.value == b'`[.0o.0mu.1mi]', (word, tag.value)
    for word, tag in (('عموم', "`[.0o.1m'u:'m]"), ('سوئد', "`[.0s'u:'.1wed]")):
        assert convert(word) == tag, (word, convert(word))
    # Apply the same policy after a compound has acquired its final stress,
    # rather than only before suffixes and constituents are assembled.
    assert convert('به‌روزرسانی‌های') == "`[.0b'e:'.0ruz.0re.0sa.0ni.1ha.0je]"

    comparisons = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100):
                    for voice_engine in (engine, old):
                        if voice_engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                voice_engine.set_parameter(key, value)
                    for long_tag, short_tag, index in (
                        ("`[.0o.0m'u:'.1mi]", '`[.0o.0mu.1mi]', 2),
                        ("`[.0zA.0r'u:'.1ri]", '`[.0zA.0ru.1ri]', 3),
                    ):
                        new = capture(engine, scope, short_tag)
                        if old:
                            previous_long = capture(old, old_scope, long_tag)
                            previous_short = capture(old, old_scope, short_tag)
                            assert ms(previous_short, index) < ms(new, index) < ms(previous_long, index), (
                                long_tag, rate, gender, speed, ms(previous_short, index),
                                ms(new, index), ms(previous_long, index))
                            # An explicitly typed long vowel retains its
                            # original audio, including the accepted M/N.
                            assert capture(engine, scope, long_tag).pcm == previous_long.pcm, (
                                long_tag, rate, gender, speed, 'explicit phonetic length changed')
                            for column in (9, 13, 15, 17):
                                assert abs(at_fraction(new, index, .5, column) -
                                           at_fraction(previous_long, index, .5, column)) <= 100, (
                                    long_tag, rate, gender, speed, column, 'vowel colour changed')
                        comparisons += 1
                    if old:
                        for tag in ('`[.1do]', '`[.1si]', '`[.1Aks]', '`[.1Ak.0si]',
                                    "`[.0s'u:'.1wed]",
                                    "`[.0'i:'.1r'a:'n]", "`[.0o.1m'u:'m]",
                                    '`[.0mu.1ri]', '`[.0o.0mu] `[.1ri]'):
                            assert capture(engine, scope, tag).pcm == capture(old, old_scope, tag).pcm, (
                                tag, rate, gender, speed, 'unrelated duration changed')
                        assert capture(engine, scope, '`[.0o.0mu.1mi]', False).pcm == \
                               capture(old, old_scope, '`[.0o.0mu.1mi]', False).pcm, (
                                   rate, gender, speed, 'Finnish changed')
    finally:
        engine.close()
        if old:old.close()

    # Exercise the full Persian and Pinglish synthesis planner without an
    # available eSpeak and confirm both input modes yield identical sound.
    for written, roman in (('عمومی', 'omumi'), ('خصوصی', 'khosusi'),
                           ('تنظیمات عمومی', 'tanzimaat omumi')):
        audio = []
        for language, text in ((PERSIAN_PROBE.id, written), (PINGLISH_PROBE.id, roman)):
            owner = make_language_engine(args.library, args.library, None,
                                         Path('/unavailable-espeak'), Path('/unavailable'), language=language)
            try:
                owner.set_parameter(SAMPLE_RATE, 3)
                owner.set_parameter(NATIVE_SPEED, 82)
                pcm = bytearray()
                assert owner.speak((text,), pcm.extend, lambda _: None) and pcm
                frontend = owner._activate(owner._key(language, '1'))._frontend
                rendered = project((text,), frontend, allow_fallback=False,
                                   pinglish=(language == PINGLISH_PROBE.id))
                assert rendered.tagged == convert(written), (text, rendered.tagged)
                audio.append(bytes(pcm))
            finally:owner.close()
        assert audio[0] == audio[1], (written, roman, 'script modes sounded different')
    print(f'Passed {comparisons} native medial U, explicit-phone and script-mode comparisons.')


if __name__ == '__main__':
    main()
