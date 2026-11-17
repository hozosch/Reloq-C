"""Keep English Unicode names and source indexes in mixed Persian text."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage
from hybrid_fa_frontend import PersianAnalysis
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine
from text_frontend import SpeechPart


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, None,
                                  args.espeak_library, args.data_parent)
    frontend = engine._engines[PERSIAN_PROBE.id]._frontend
    original = frontend.analyze
    frontend.analyze = lambda value, **kwargs: PersianAnalysis(
        (SpeechPart('fa', '`[.1ma]'),), None, True)

    def render(parts):
        pcm, indexes = bytearray(), []
        assert engine.speak(parts, pcm.extend, lambda value: indexes.append((value, len(pcm))))
        assert pcm
        return bytes(pcm), indexes

    try:
        for char, name in (('\0', 'symbol for null'),
                           ('\ud800', 'replacement character'),
                           ('\u202e', 'right-to-left override')):
            # An uncovered Persian word requires reference validation. The
            # Unicode name must not be passed into the Persian phone parser.
            before = (EciIndex(1), 'پدیده‌ای ', EciIndex(2), char + ' سلام', EciIndex(3))
            explicit = (EciIndex(1), 'پدیده‌ای ', EciIndex(2),
                        EciLanguage(US_ENGLISH.id, '1'), name,
                        EciLanguage(PERSIAN_PROBE.id, '1'), ' سلام', EciIndex(3))
            assert render(before) == render(explicit), ascii(char)
        engine.select_voice(PINGLISH_PROBE.id, '1')
        before = (EciIndex(4), 'salam ', EciIndex(5), '\0 donya', EciIndex(6))
        explicit = (EciIndex(4), 'salam ', EciIndex(5),
                    EciLanguage(US_ENGLISH.id, '1'), 'symbol for null',
                    EciLanguage(PINGLISH_PROBE.id, '1'), ' donya', EciIndex(6))
        assert render(before) == render(explicit)
    finally:
        frontend.analyze = original
        engine.close()
    print('Passed mixed Persian/Pinglish Unicode names, recovery and indexes.')


if __name__ == '__main__':
    main()
