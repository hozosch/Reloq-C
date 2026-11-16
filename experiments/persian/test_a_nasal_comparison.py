"""Ensure the alternate back-A scope changes no unrelated tested voice."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from test_clarity_candidate import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    for dialect in (0x10000, 0x40000, 0x90000):
        old = DirectEciEngine(args.previous.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        new = DirectEciEngine(args.candidate.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            for voice in (old, new):
                voice.set_parameter(PROCESS_TAGS, 1)
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for voice in (old, new):
                        for param, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, 98)):
                            voice.set_parameter(param, value)
                    for text in ('hello', 'Salam?', '`[.1li]', '`[.1pe]'):
                        assert render(old, text) == render(new, text), ('ordinary voice changed', dialect, text)
                        checks += 1
                    if dialect == 0x90000:
                        for text in ('`[.1li]', '`[.1pe]', '`[.1hAs.0tim]'):
                            assert render(old, text, persian=True) == render(new, text, persian=True), (
                                'unrelated Persian sound changed', rate, gender, text)
                            checks += 1
                        for text in ('`[.1pa]', '`[.0sA.1lam]'):
                            before = render(old, text, persian=True)
                            after = render(new, text, persian=True)
                            assert before != after and len(before) == len(after), (
                                'back-A contrast missing or timing changed', rate, gender, text)
                            checks += 1
        finally:
            old.close()
            new.close()
    print(f'Passed {checks} back-A contrast, L and other-language isolation checks.')


if __name__ == '__main__':
    main()
