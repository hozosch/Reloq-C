"""Isolate reported Italian apostrophe crashes while probing the ECI core."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

from _reloq_eci_direct import DirectEciEngine
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--child')
    args = parser.parse_args()
    if args.child is not None:
        engine = DirectEciEngine(args.library.resolve(), dialect=0x50000,
                                 encoding='utf8', native_voice=True)
        try:
            pcm, _indices = render(engine, [args.child])
            print(json.dumps({'pcm_bytes': len(pcm), 'phones': engine._phonemes_for_probe(args.child)}))
            if not pcm:
                raise RuntimeError('No speech produced')
        finally:
            engine.close()
        return
    cases = ('ciao', 'po', "po'", 'po\u2019', "Aspetta un po'", 'Aspetta un po\u2019',
             "un po' di latte", "l'amico", "perche'", 'cos\u00ec')
    for case in cases:
        command = [sys.executable, __file__, '--library', str(args.library), '--child', case]
        try:
            result = subprocess.run(command, capture_output=True, text=True,
                                    encoding='utf8', errors='replace', timeout=15)
            print(json.dumps({'text': case, 'exit': result.returncode,
                              'stdout': result.stdout[-1500:], 'stderr': result.stderr[-1500:]},
                             ensure_ascii=True), flush=True)
        except subprocess.TimeoutExpired:
            print(json.dumps({'text': case, 'timeout': True}, ensure_ascii=True), flush=True)


if __name__ == '__main__':
    main()
