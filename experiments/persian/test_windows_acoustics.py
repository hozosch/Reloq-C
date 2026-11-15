"""Test the private Windows probe's ordinary and scoped acoustic paths."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PROCESS_TAGS, SAMPLE_RATE
from direct_fa_sequence import ScopedDirectPersianEngine
from hybrid_fa_frontend import DirectFirstPersian


def render(library, dialect, text, *, scoped=None, rate=3, gender=0, onsets=()):
    voice = DirectEciEngine(library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    try:
        voice.set_parameter(PROCESS_TAGS, 1)
        voice.set_parameter(SAMPLE_RATE, rate)
        voice.set_parameter(GENDER, gender)
        if scoped is not None:
            import ctypes
            scope = voice._dll.reloq_fa_acoustic_scope
            scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
            previous = scope(int(scoped))
            assert previous == 0
            if scoped:
                onset = voice._dll.reloq_fa_native_onsets
                onset.argtypes, onset.restype = (ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t), ctypes.c_int
                selected = (ctypes.c_uint32 * len(onsets))(*onsets) if onsets else None
                assert onset(selected, len(onsets)) == 0
        pcm = bytearray()
        assert voice.speak((text,), pcm.extend, lambda _: None) and pcm
        if scoped is not None:
            if scoped:
                assert onset(None, 0) == 0
            assert scope(previous) == int(scoped)
        return bytes(pcm)
    finally:
        voice.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    args = parser.parse_args()
    pairs = ((0x90000, 'viisi.'), (0x10000, 'father and friends.'),
             (0x40000, 'Guten Tag.'))
    for rate in (1, 3, 6):
        for gender in (0, 1):
            for dialect, text in pairs:
                original = render(args.baseline, dialect, text, rate=rate, gender=gender)
                assert render(args.candidate, dialect, text, scoped=False, rate=rate,
                              gender=gender) == original
                if dialect != 0x90000:
                    assert render(args.candidate, dialect, text, scoped=True, rate=rate,
                                  gender=gender) == original
    tagged = '`[.0sA.1lam] `[.1do.0nja]?'
    finnish = render(args.candidate, 0x90000, tagged, scoped=False)
    persian = render(args.candidate, 0x90000, tagged, scoped=True)
    assert finnish != persian
    onset_text = '`[.1so] `[.0al]'
    plain = render(args.candidate, 0x90000, onset_text, scoped=True)
    marked = render(args.candidate, 0x90000, onset_text, scoped=True, onsets=(2,))
    assert plain != marked
    assert render(args.candidate, 0x90000, onset_text, scoped=True) == plain
    with ThreadPoolExecutor(max_workers=2) as pool:
        jobs = [pool.submit(render, args.candidate, 0x90000, tagged, scoped=active)
                for active in (False, True)]
        assert [job.result() for job in jobs] == [finnish, persian]
    unused = Path('/nonexistent/reloq-reference')
    with DirectFirstPersian(args.candidate, unused, unused, unused) as frontend:
        voices = (ScopedDirectPersianEngine(args.candidate, frontend),
                  ScopedDirectPersianEngine(args.candidate, frontend, pinglish=True))
        try:
            outputs = []
            for voice, text in zip(voices, ('سلام دنیا؟', 'salam donya?')):
                pcm = bytearray()
                assert voice.speak((text,), pcm.extend, lambda _: None) and pcm
                outputs.append(bytes(pcm))
            assert outputs[0] == outputs[1] and frontend._worker is None
        finally:
            for voice in voices:
                voice.close()
    print('Passed Windows scoped acoustics, donor isolation, threading and direct Pinglish.')


if __name__ == '__main__':
    main()
