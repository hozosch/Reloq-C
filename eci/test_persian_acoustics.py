"""Compare common-core Persian acoustics with the accepted private build."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes
from pathlib import Path
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PROCESS_TAGS, SAMPLE_RATE


def render(library, dialect, text, *, selected, rate=3, gender=0):
    voice = DirectEciEngine(library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    try:
        for setting, value in ((PROCESS_TAGS, 1), (SAMPLE_RATE, rate), (GENDER, gender)):
            voice.set_parameter(setting, value)
        scope = (getattr(voice._dll, 'eciPersianAcousticScope', None) or
                 voice._dll.reloq_fa_acoustic_scope)
        scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
        previous = scope(int(selected))
        assert previous == 0
        try:
            pcm, marks = bytearray(), []
            assert voice.speak((text,), pcm.extend, marks.append)
            return bytes(pcm), marks
        finally:
            assert scope(previous) == int(selected)
    finally:
        voice.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    args = parser.parse_args()
    cases = ((0x10000, 'The six ships are ready.', False),
             (0x40000, 'Sechs silberne Tassen.', False),
             (0x90000, 'Viisi sanaa.', False),
             (0x20000, 'Hola, buenos dias.', False),
             (0x60000, '你好世界。', False),
             (0x70000, 'Bom dia.', False),
             (0x120000, 'Καλημέρα κόσμε.', False),
             (0x90000, '`[.0sA.1lam] `[.1do.0nja]?', True),
             (0x90000, '`[.1GoS] `[.1SeS] `[.1Ga.0reJ]', True),
             (0x90000, '`[.1noR.0te] `[.1em.0kan]', True))
    checks = 0
    for dialect, text, active in cases:
        for rate in (0, 1, 2, 3, 4, 5, 6):
            for gender in (0, 1):
                actual = render(args.library, dialect, text, selected=active, rate=rate, gender=gender)
                expected = render(args.reference, dialect, text, selected=active, rate=rate, gender=gender)
                assert actual == expected, (dialect, text, rate, gender)
                checks += 1
    text = '`[.0sA.1lam] `[.1do.0nja]?'
    expected = [render(args.library, 0x90000, text, selected=value) for value in (False, True)]
    assert expected[0] != expected[1]
    barrier = threading.Barrier(2)
    def worker(value):
        barrier.wait(timeout=15)
        return render(args.library, 0x90000, text, selected=value)
    with ThreadPoolExecutor(max_workers=2) as pool:
        futures = [pool.submit(worker, value) for value in (False, True)]
        assert [future.result() for future in futures] == expected
    print(f'Passed {checks} native acoustic comparisons and thread isolation.')


if __name__ == '__main__':
    main()
