"""Ensure a queued voice change never consumes the following number."""
from __future__ import annotations

import argparse
from pathlib import Path
from test_eci_inline_queue import Engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    for language, prefix, suffix in ((0x10000, 'Batteries, ', '90 percent charged.'),
                                     (0x40000, 'Batterien, ', '90 Prozent geladen.')):
        engine = Engine(args.library, language)
        try:
            for direct in (False, True):
                engine.direct = direct
                for legacy in (False, True):
                    for tags in (0, 1):
                        for param, name, before, after in (
                            (0, 'vg', 0, 1), (1, 'vh', 50, 80),
                            (2, 'vb', 60, 65), (3, 'vf', 30, 70),
                            (4, 'vr', 0, 30), (5, 'vy', 0, 40),
                            (6, 'vs', 50, 150), (7, 'vv', 100, 30),
                        ):
                            engine.tags(1); engine.voice(param, before)
                            engine.add(prefix, legacy)
                            assert engine.lib.eciInsertIndex(engine.h, 11)
                            engine.add(f'`{name}{after} {suffix}', legacy)
                            assert engine.lib.eciInsertIndex(engine.h, 12)
                            expected = engine.finish(); expected_marks = list(engine.indices)

                            engine.tags(tags); engine.voice(param, before)
                            engine.add(prefix, legacy)
                            assert engine.lib.eciInsertIndex(engine.h, 11)
                            engine.voice(param, after)
                            engine.add(suffix, legacy)
                            assert engine.lib.eciInsertIndex(engine.h, 12)
                            actual = engine.finish(); actual_marks = list(engine.indices)
                            assert actual and actual == expected, (
                                hex(language), direct, legacy, tags, name,
                                len(actual), len(expected))
                            # A generated control is placed after a source index,
                            # whereas the explicit tag is part of caller text;
                            # provider callback timing for that boundary can differ.
                            assert [mark for mark, _ in actual_marks] == [11, 12]
                            assert 0 < actual_marks[0][1] <= actual_marks[1][1] == len(actual)
                            checks += 1
            # Also cover controls generated from WPM and Hertz units, as
            # well as several changed parameters at the same source position.
            engine.direct = False
            engine.tags(1)
            assert engine.lib.eciSetParam(engine.h, 0x1017, 150) >= 0
            engine.add(prefix); engine.add('`vwpm300 ' + suffix)
            expected = engine.finish()
            engine.tags(0)
            assert engine.lib.eciSetParam(engine.h, 0x1017, 150) >= 0
            engine.add(prefix)
            assert engine.lib.eciSetParam(engine.h, 0x1017, 300) >= 0
            engine.add(suffix)
            assert engine.finish() == expected, (hex(language), 'wpm')
            assert engine.lib.eciSetParam(engine.h, 0x1017, 0) >= 0
            checks += 1

            assert engine.lib.eciSetParam(engine.h, 8, 1) >= 0
            engine.tags(1); engine.voice(2, 150)
            engine.add(prefix); engine.add('`vbhz220 ' + suffix)
            expected = engine.finish()
            engine.tags(0); engine.voice(2, 150); engine.add(prefix)
            engine.voice(2, 220); engine.add(suffix)
            assert engine.finish() == expected, (hex(language), 'hertz')
            assert engine.lib.eciSetParam(engine.h, 8, 0) >= 0
            checks += 1

            engine.tags(1)
            for parameter, value in ((0, 0), (2, 60), (7, 100)):
                engine.voice(parameter, value)
            engine.add(prefix); engine.add('`vg1`vb65`vv30 ' + suffix)
            expected = engine.finish()
            engine.tags(0)
            for parameter, value in ((0, 0), (2, 60), (7, 100)):
                engine.voice(parameter, value)
            engine.add(prefix)
            for parameter, value in ((0, 1), (2, 65), (7, 30)):
                engine.voice(parameter, value)
            engine.add(suffix)
            assert engine.finish() == expected, (hex(language), 'combined')
            checks += 1
        finally:
            engine.close()
    print(f'Passed {checks} digit-leading ECI voice changes and source-index checks.')


if __name__ == '__main__':
    main()
