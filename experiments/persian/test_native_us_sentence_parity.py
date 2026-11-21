"""Compare native Persian sentence rendering with the active NVDA planner.

Both routes use the same selected voice and work with no eSpeak executable,
DLL or data. Compare complete PCM and the sample positions of source marks.
"""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import random
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, SAMPLE_RATE
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    library = args.library.resolve()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(library))
    H, I, S = c.c_void_p, c.c_int, c.c_size_t
    callback = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciGetParam', I, (H, I)), ('eciSetParam', I, (H, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_int16))),
        ('eciPersianSpeakUsNativeUtf8', I, (H, H, c.c_char_p, S, I, c.POINTER(S),
                                         c.POINTER(I), S, S, c.POINTER(S))),
    )
    for name, result, parameters in functions:
        fn = getattr(dll, name); fn.restype, fn.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback, H)
    voice, pilot = dll.eciNewEx(0x90000), dll.eciNewEx(0x10000)
    assert voice and pilot
    buffer, pilot_buffer = (c.c_int16 * 2048)(), (c.c_int16 * 2048)()
    pcm, indices, pilot_audio = bytearray(), [], []

    @callback
    def receive(_handle, message, value, _context):
        if message == 0 and value:
            pcm.extend(c.string_at(buffer, value * 2))
        elif message == 2:
            indices.append((value, len(pcm) // 2))
        return 1

    @callback
    def silent(_handle, message, value, _context):
        if message == 0 and value:
            pilot_audio.append(value)
        return 1

    for handle, output, cb in ((voice, buffer, receive), (pilot, pilot_buffer, silent)):
        dll.eciRegisterCallback(handle, cb, None)
        assert dll.eciSetOutputBuffer(handle, len(output), output) == 1
    cases = (
        (PERSIAN_PROBE, 0, ('سلام', 'سلام دنیا.', 'کتاب را.', 'سلام، دنیا؟',
                            'من (دنیا) خوب', 'ژژژژژ', 'سوال', 'تعجب', 'او آمد.', '۱۲')),
        (PINGLISH_PROBE, 1, ('salaam', 'salaam donyaa.', 'ketaab raa.', 'salam, donya?',
                             'salaam (donyaa) khoob', 'aks', 'aksi', 'u aamad.', '12',
                             'tajjub', 'salaam aamad tajjub.', 'tanzimaat tajjub in raa?',
                             'do tajjub omumi gozine khoone?')),
    )
    rng = random.Random(411)
    fa_words = ('سلام', 'دنیا', 'کتاب', 'را', 'است', 'من', 'خانه', 'او', 'آمد', 'برای',
                'این', 'دو', 'سوال', 'تعجب', 'گزینه', 'خوب', 'ایران', 'تنظیمات', 'عمومی', 'ژژژژژ')
    pinglish_words = ('salaam', 'donyaa', 'ketaab', 'raa', 'ast', 'man', 'khoone', 'u', 'aamad',
                      'baraaye', 'in', 'do', 'soal', 'tajjub', 'gozine', 'khoob', 'iraan', 'tanzimaat', 'omumi')
    def corpus(words):
        return tuple(' '.join(rng.choice(words) for _ in range(rng.randrange(2, 6))) + rng.choice(('.', '?', ''))
                     for _ in range(100))
    generated = {PERSIAN_PROBE.id: corpus(fa_words), PINGLISH_PROBE.id: corpus(pinglish_words)}
    checks = 0
    with tempfile.TemporaryDirectory(prefix='reloq-native-fa-parity-') as temp:
        root = Path(temp)
        owner = make_language_engine(library, library, None, root / 'no-espeak-library', root,
                                     language=PERSIAN_PROBE.id)
        try:
            for profile, mode, texts in cases:
                for rate in (1, 3, 6):
                    for preset in ('1', '2'):
                        owner.select_voice(profile.id, preset)
                        owner.set_parameter(SAMPLE_RATE, rate)
                        owner.set_parameter(0x1007, 85)
                        fa = owner._activate(owner._key(profile.id, preset))
                        us = owner._activate(owner._key(US_ENGLISH.id, preset))
                        assert not fa._frontend._reference_available
                        for handle, engine in ((voice, fa), (pilot, us)):
                            source = engine._voice if hasattr(engine, '_voice') else engine
                            for parameter in (5, 0x1007, 0x1008, 0x1009, 0x100a, 0x100b,
                                              0x100c, 0x100d, 0x100e, 0x100f, 0x1010, 0x1011, 0x1015):
                                value = dll.eciGetParam(source._handle, parameter)
                                if value >= 0:
                                    assert dll.eciSetParam(handle, parameter, value) >= 0
                        requests = [(text, (False, True)) for text in texts]
                        if rate == 3 and preset == '1':
                            requests.extend((text, (True,)) for text in generated[profile.id])
                        for text, indexes_to_test in requests:
                            for indexed in indexes_to_test:
                                expected, expected_indices = bytearray(), []
                                mid = len(text) // 2
                                parts = ((EciIndex(90), text[:mid], EciIndex(91), text[mid:], EciIndex(92))
                                         if indexed else (text,))
                                assert owner.speak(parts, expected.extend,
                                    lambda value: expected_indices.append((value, len(expected) // 2)))
                                raw = text.encode('utf8'); error = S(); pcm.clear(); indices.clear()
                                offsets = (S * 3)(0, len(text[:mid].encode('utf8')), len(raw)) if indexed else None
                                values = (I * 3)(90, 91, 92) if indexed else None
                                status = dll.eciPersianSpeakUsNativeUtf8(voice, pilot, raw, len(raw), mode,
                                            offsets, values, 3 if indexed else 0, 30000, c.byref(error))
                                assert status == 0, (profile.locale, rate, preset, text, indexed, status, error.value)
                                assert bytes(pcm) == bytes(expected), (profile.locale, rate, preset, text, indexed,
                                                                       len(pcm), len(expected))
                                assert indices == expected_indices, (profile.locale, rate, preset, text, indexed,
                                                                     indices, expected_indices)
                                assert not pilot_audio
                                checks += 1
            # Compare the new complete Pinglish bracket analysis with the
            # previous no-reference fallback, including marks inside words.
            owner.select_voice(PINGLISH_PROBE.id, '1')
            owner.set_parameter(SAMPLE_RATE, 3)
            frontend = owner._activate(owner._key(PINGLISH_PROBE.id, '1'))._frontend
            complete = frontend._project_text_complete
            assert complete is not None and not frontend._reference_available
            for text in ('salam (iran)', 'salaam (donyaa) khoob', 'man (donya) khub',
                         'salam [iran] donya', 'salam {iran} donya', 'u (aamad) tajjub'):
                middle = len(text) // 2
                parts = (EciIndex(1), text[:middle], EciIndex(2), text[middle:], EciIndex(3))
                def render():
                    output, marks = bytearray(), []
                    assert owner.speak(parts, output.extend, lambda value: marks.append((value, len(output))))
                    return bytes(output), marks
                try:
                    frontend._project_text_complete = None
                    previous = render()
                finally:
                    frontend._project_text_complete = complete
                assert render() == previous, text
                checks += 1
        finally:
            owner.close(); dll.eciDelete(voice); dll.eciDelete(pilot)
    print(f'Passed {checks} native/current Persian and Pinglish PCM and index comparisons without eSpeak.')


if __name__ == '__main__':
    main()
