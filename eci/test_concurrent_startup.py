"""Start independent ECI voices concurrently in fresh processes."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import subprocess
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine


def child(library):
    cases = ((0x10000, 'hello world'), (0x10000, 'open settings'),
             (0x40000, 'Guten Tag'), (0x90000, 'Viisi sanaa'),
             (0x20000, 'buenos dias'))
    barrier = threading.Barrier(len(cases))
    def run(case):
        dialect, text = case
        voice = DirectEciEngine(library, dialect=dialect, encoding='utf8', native_voice=True)
        try:
            barrier.wait(timeout=20)
            assert voice._phonemes_for_probe(text)
            pcm = bytearray()
            assert voice.speak((text,), pcm.extend, lambda _: None) and pcm
        finally:
            voice.close()
    with ThreadPoolExecutor(max_workers=len(cases)) as executor:
        futures = [executor.submit(run, case) for case in cases]
        for future in futures:
            future.result(timeout=30)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=8)
    parser.add_argument('--child', action='store_true')
    args = parser.parse_args()
    if args.child:
        child(args.library.resolve())
        return
    for index in range(args.rounds):
        result = subprocess.run([sys.executable, '-B', __file__, '--library', str(args.library.resolve()),
                                 '--child'], capture_output=True, text=True, timeout=45)
        if result.returncode:
            raise RuntimeError(f'Concurrent startup failed in round {index + 1}: '
                               f'{result.returncode}\n{result.stdout}\n{result.stderr}')
    print(f'Passed {args.rounds} fresh concurrent multilingual voice starts.')


if __name__ == '__main__':
    main()
