"""Exercise English transition fixes through the real public ECI audio path.

The previous release is a control for unaffected contexts; the cases that
need human listening must produce a change without moving index boundaries.
"""
import argparse
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, NATIVE_SPEED,
                               PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import render


def pair(library, reference, dialect, dictionary):
    engines = [DirectEciEngine(path.resolve(), dialect=dialect,
                               encoding='utf8', native_voice=True)
               for path in (reference, library)]
    for engine in engines:
        engine.set_parameter(PROCESS_TAGS, 1)
        if dictionary:
            engine.set_dictionary_path(dictionary.resolve())
    return engines


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--dictionary', type=Path)
    args = parser.parse_args()
    checks = 0
    for dialect in (0x10000, 0x10001):
        baseline, candidate = pair(args.library, args.reference, dialect, args.dictionary)
        try:
            sh_cases = ('that shroom', 'that shrowd', 'that shred',
                        'that shreck', 'that shrike', 'that shreek', 'antshrike')
            unaffected = ('shroom', 'the shroom', 'mushroom', 'that shoe',
                          'that shallow', 'fresh red', 'Iowa, awaits',
                          'awaits', 'go away', 'Iowa Iowa', 'idea about', 'sofa about')
            if args.dictionary and dialect == 0x10000:
                for engine in (baseline, candidate):
                    assert 'SrYk' in engine._phonemes_for_probe('antshrike')
                    checks += 1
            for rate in (1, 3):
                for speed in (50, 75):
                    for engine in (baseline, candidate):
                        engine.set_parameter(SAMPLE_RATE, rate)
                        engine.set_parameter(NATIVE_SPEED, speed)
                    for text in sh_cases:
                        before = render(baseline, [text, EciIndex(71)])
                        after = render(candidate, [text, EciIndex(71)])
                        assert before[0] and before[0] != after[0], (dialect, rate, speed, text)
                        assert len(before[0]) == len(after[0]) and before[1] == after[1], (dialect, text)
                        checks += 2
                    for text in unaffected:
                        assert render(candidate, [text, EciIndex(71)]) == render(
                            baseline, [text, EciIndex(71)]), (dialect, rate, speed, text)
                        checks += 1
                    if dialect == 0x10000:
                        for text in ('Iowa awaits', 'Iowa away', 'Iowa alone', 'sofa away'):
                            before = render(baseline, [text, EciIndex(71)])
                            after = render(candidate, [text, EciIndex(71)])
                            assert before[0] and before[0] != after[0], (rate, speed, text)
                            assert len(before[0]) == len(after[0]) and before[1] == after[1], text
                            checks += 2
        finally:
            baseline.close()
            candidate.close()
    print(f'English transitions: {checks} real ECI audio, dictionary and index checks passed')


if __name__ == '__main__':
    main()
