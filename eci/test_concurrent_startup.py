"""Start independent ECI voices concurrently in fresh processes."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine


def child(library, other=None, serial=False):
    cases = ((0x10000, 'hello world'), (0x10000, 'open settings'),
             (0x40000, 'Guten Tag'), (0x90000, 'Viisi sanaa'),
             (0x20000, 'buenos dias'))
    barrier = threading.Barrier(len(cases))
    def run(index, case):
        dialect, text = case
        selected = other if other is not None and index % 2 else library
        voice = DirectEciEngine(selected, dialect=dialect, encoding='utf8', native_voice=True)
        try:
            if not serial:
                barrier.wait(timeout=20)
            try:
                shown = voice._phonemes_for_probe(text)
            except BaseException as error:
                status = tuple(voice._dll.eciGetParam(voice._handle, key)
                               for key in (0x1000, 0x1001, 0x1002))
                raise RuntimeError(f'phoneme analysis failed: language={dialect:#x}, '
                                   f'text={text!r}, diagnostics={status}') from error
            assert shown, (dialect, text)
            pcm = bytearray()
            assert voice.speak((text,), pcm.extend, lambda _: None) and pcm
        finally:
            voice.close()
    if serial:
        for index, case in enumerate(cases):
            run(index, case)
    else:
        with ThreadPoolExecutor(max_workers=len(cases)) as executor:
            futures = [executor.submit(run, index, case) for index, case in enumerate(cases)]
            for future in futures:
                future.result(timeout=30)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=8)
    parser.add_argument('--child', action='store_true')
    parser.add_argument('--serial', action='store_true')
    parser.add_argument('--other-library', type=Path)
    args = parser.parse_args()
    if args.child:
        child(args.library.resolve(), args.other_library.resolve() if args.other_library else None, args.serial)
        return
    with tempfile.TemporaryDirectory(prefix='reloq-eci-copy-') as folder:
        copy = Path(folder) / ('second_' + args.library.name)
        shutil.copyfile(args.library, copy)
        serial = subprocess.run([sys.executable, '-B', __file__, '--library', str(args.library.resolve()),
                                 '--child', '--serial'], capture_output=True, text=True, timeout=45)
        if serial.returncode:
            raise RuntimeError(f'Serial voice baseline failed: {serial.returncode}\n'
                               f'{serial.stdout}\n{serial.stderr}')
        for index in range(args.rounds):
            for additional in ((), ('--other-library', str(copy))):
                result = subprocess.run([sys.executable, '-B', __file__, '--library', str(args.library.resolve()),
                                         '--child', *additional], capture_output=True, text=True, timeout=45)
                if result.returncode:
                    raise RuntimeError(f'Concurrent startup failed in round {index + 1}, '
                                       f'copies={bool(additional)}: {result.returncode}\n'
                                       f'{result.stdout}\n{result.stderr}')
    print(f'Passed {args.rounds} fresh starts with one and with two concurrent library copies.')


if __name__ == '__main__':
    main()
