"""Compare real playback before and after moving words into native C."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'previous-library', 'native-library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, required=True, type=Path)
    args = parser.parse_args()
    old = make_language_engine(args.scope_probe, args.previous_library, None,
                               args.espeak_library, args.data_parent)
    new = make_language_engine(args.scope_probe, args.native_library, None,
                               args.espeak_library, args.data_parent)
    cases = (('درسته',), ('برگزیده',), ('الکترونیکی',), ('همگام‌سازی',),
             ('این درسته و برگزیده الکترونیکی است.',),
             ('بله این حرف شما - درسته حتما',),
             (EciIndex(1), 'NVDA ', EciIndex(2), 'برگزیده', EciIndex(3), ' سلام', EciIndex(4)))
    try:
        for voice in (old, new):
            for key, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65),
                               (NATIVE_PITCH, 65), (NATIVE_INFLECTION, 30)):
                voice.set_parameter(key, value)
        for parts in cases:
            results = []
            for voice in (old, new):
                audio, marks = bytearray(), []
                assert voice.speak(parts, audio.extend, lambda index: marks.append((index, len(audio))))
                results.append((bytes(audio), marks))
            assert results[0][0] and results[0] == results[1], parts
        assert old._engines[PERSIAN_PROBE.id]._frontend._worker is not None
        assert new._engines[PERSIAN_PROBE.id]._frontend._worker is None
    finally:
        old.close()
        new.close()
    print(f'Passed {len(cases)} unchanged playback and source-index comparisons; new readings use no eSpeak.')


if __name__ == '__main__':
    main()
