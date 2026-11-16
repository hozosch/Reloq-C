"""Compare the scoped timing candidate to the previous shared C engine."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE


def render(voice, text, *, persian=False):
    scope = getattr(voice._dll, 'eciPersianAcousticScope')
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    previous = scope(int(persian))
    if previous < 0:
        raise RuntimeError('cannot select the acoustic scope')
    audio = bytearray()
    try:
        assert voice.speak((text,), audio.extend, lambda _index: None)
        return bytes(audio)
    finally:
        assert scope(previous) >= 0


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
            for rate, gender, speed in ((1, 0, 40), (3, 0, 98), (6, 1, 140)):
                for voice in (old, new):
                    for param, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                        voice.set_parameter(param, value)
                for text in ('hello', 'Salam?', '`[.1lam]', '`[.1pA]', '`[.1do.0nja]'):
                    assert render(old, text) == render(new, text), ('unscoped voice changed', dialect, text)
                    checks += 1
                if dialect == 0x90000:
                    # Preserve a stressed L context, a stressed standalone
                    # vowel and a word-final unstressed vowel exactly.
                    for text in ('`[.1lam]', '`[.1pA]', '`[.1do.0nja]',
                                 '`[.1do.0nja] `[.1pa]', '`[.1lam] `[.1li]'):
                        assert render(old, text, persian=True) == render(new, text, persian=True), (
                            'protected scoped context changed', rate, gender, speed, text)
                        checks += 1
                    text = '`[.0sA.1lam]'
                    previous = render(old, text, persian=True)
                    candidate = render(new, text, persian=True)
                    assert candidate != previous and len(candidate) < len(previous), (
                        'unstressed internal vowel did not shorten', rate, gender, speed,
                        len(previous), len(candidate))
                    checks += 1
        finally:
            old.close()
            new.close()
    print(f'Passed {checks} scoped timing, protected L/vowel and other-language checks.')


if __name__ == '__main__':
    main()
