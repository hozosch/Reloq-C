"""Verify reference pauses use Finnish's own vowel onset, scoped per job."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes
from pathlib import Path

from direct_fa_sequence import EciIndex, EciPitch, ScopedDirectPersianEngine
from hybrid_fa_frontend import DirectFirstPersian
from listening import render
from test_selected_probe import engine, set_confirmed_persian_probe
from _reloq_eci_direct import GENDER, SAMPLE_RATE


def configured(voice, rate, gender):
    voice.set_parameter(SAMPLE_RATE, rate)
    voice.set_parameter(GENDER, gender)


def manual(library, text, words, *, rate=3, gender=0):
    voice = engine(library, 0x90000)
    try:
        configured(voice, rate, gender)
        scope = voice._dll.reloq_fa_acoustic_scope
        onset = voice._dll.reloq_fa_native_onsets
        scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
        onset.argtypes, onset.restype = (ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t), ctypes.c_int
        previous = scope(1)
        assert previous == 0
        try:
            marks = (ctypes.c_uint32 * len(words))(*words) if words else None
            assert onset(marks, len(words)) == 0
            return render(voice, text)
        finally:
            assert onset(None, 0) == 0
            assert scope(previous) == 1
    finally:
        voice.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scope-probe', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    samples = (('سوال', '`[.1so] `[.0al]'),
               ('بنابراین', "`[.2bA.0n'a:'.1bAr] `[.0in]"),
               ('بدبخت سوال', '`[.1bAd.0bAGt] `[.1so] `[.0al]'),
               ('سوال سوال', '`[.1so] `[.0al] `[.1so] `[.0al]'))
    reference = engine(args.reference, 0x90000)
    with DirectFirstPersian(args.native_library, args.worker,
                            args.espeak_library, args.data_parent) as frontend:
        candidate = ScopedDirectPersianEngine(args.scope_probe, frontend, allow_fallback=True)
        try:
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    configured(reference, rate, gender)
                    configured(candidate, rate, gender)
                    for source, tagged in samples:
                        # This is the exact ordinary Finnish onset rule with
                        # no custom glottal-stop carrier in the phone stream.
                        set_confirmed_persian_probe(no_auto_glottal=0, d=1, z=1)
                        expected = render(reference, tagged)
                        pcm, indices = bytearray(), []
                        assert candidate.speak((EciIndex(1), source, EciIndex(2)), pcm.extend, indices.append)
                        assert bytes(pcm) == expected and indices == [1, 2], (source, rate, gender)
                        set_confirmed_persian_probe(no_auto_glottal=1, d=1, z=1)
                        assert expected != render(reference, tagged), (source, rate, gender)
            # An onset selection cannot leak into the following request.
            first, second = bytearray(), bytearray()
            assert candidate.speak(('سلام دنیا.',), first.extend, lambda _: None)
            assert candidate.speak(('سوال',), lambda _: None, lambda _: None)
            assert candidate.speak(('سلام دنیا.',), second.extend, lambda _: None)
            assert first == second and first
            # NVDA indexes do not move or disable an internally marked onset.
            for source, parts in (('سوال', ('سو', EciIndex(21), 'ال')),
                                  ('سلام آیلار', ('سلام ', EciIndex(22), 'آیلار'))):
                plain, indexed, observed = bytearray(), bytearray(), []
                assert candidate.speak((source,), plain.extend, lambda _: None)
                assert candidate.speak(parts, indexed.extend, observed.append)
                assert plain == indexed and observed == [parts[1].value], source
            set_confirmed_persian_probe(no_auto_glottal=0, d=1, z=1)
            expected, actual = bytearray(), bytearray()
            assert reference.speak(('`[.0sA.1lam] ', EciPitch(70), '`[.1aj.0lar]'),
                                   expected.extend, lambda _: None)
            assert candidate.speak(('سلام ', EciPitch(70), 'آیلار'), actual.extend, lambda _: None)
            assert actual == expected and actual
            # Selecting one onset must not re-enable Finnish's automatic
            # glottalization at a later unmarked word boundary.
            configured(candidate, 3, 0)
            tagged = "`[.1so] `[.0al] `[.1'a:'.0mAd]."
            expected = manual(args.scope_probe, tagged, (2,))
            unwanted = manual(args.scope_probe, tagged, (2, 3))
            pcm = bytearray()
            assert candidate.speak(('سوال آمد.',), pcm.extend, lambda _: None)
            assert bytes(pcm) == expected and expected != unwanted
            # An explicit eSpeak glottal stop keeps the original custom phone.
            configured(reference, 3, 0)
            set_confirmed_persian_probe(no_auto_glottal=1, d=1, z=1)
            expected = render(reference, '`[.1sa.0wAt]')
            pcm = bytearray()
            assert candidate.speak(('ساعت',), pcm.extend, lambda _: None)
            assert bytes(pcm) == expected
            # Keep the reference syllable stress in one native word; an
            # invented D boundary changed its prosody without preserving D.
            expected = render(reference, '`[.1bAd.0bAGt]')
            pcm = bytearray()
            assert candidate.speak(('بدبخت',), pcm.extend, lambda _: None)
            assert bytes(pcm) == expected
        finally:
            candidate.close()
            reference.close()
    text = '`[.1so] `[.0al]'
    set_confirmed_persian_probe(no_auto_glottal=1, d=1, z=1)
    plain, marked = manual(args.scope_probe, text, ()), manual(args.scope_probe, text, (2,))
    assert plain != marked
    with ThreadPoolExecutor(max_workers=2) as pool:
        tasks = (pool.submit(manual, args.scope_probe, text, ()),
                 pool.submit(manual, args.scope_probe, text, (2,)))
        assert tuple(task.result() for task in tasks) == (plain, marked)
    print('Passed native Finnish onset parity, unsplit badbakht, scope and threading.')


if __name__ == '__main__':
    main()
