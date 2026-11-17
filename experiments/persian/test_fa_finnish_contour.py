"""Exercise the experimental Persian US contour and pitch controls."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage, NATIVE_INFLECTION, NATIVE_PITCH
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, US_ENGLISH
from continuous_driver import _project_part


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--text-library', type=Path)
    args = parser.parse_args()
    voice = make_language_engine(args.library, args.text_library or args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))

    def render(target, parts):
        pcm, marks = bytearray(), []
        assert target.speak(parts, pcm.extend, lambda index: marks.append((index, len(pcm))))
        assert pcm
        return bytes(pcm), marks

    try:
        assert getattr(voice, '_persian_korean_contour', None) is None
        persian = voice._engines[PERSIAN_PROBE.id]
        # The English host belongs to the preceding language segment; its
        # following Persian object marker still keeps the weak reading.
        joined, marks, _ = _project_part(persian, (EciIndex(6), 'را'), follows=False, preceded=True)
        isolated, _, _ = _project_part(persian, ('را',), follows=False)
        assert joined == '`00 `[.0ra]' and marks == [(0, 6)] and isolated == '`[.1ra]'
        with patch('continuous_driver.contour', side_effect=AssertionError('Korean donor was used')):
            for text in ('کتاب', 'سلام دنیا.', 'امروز من یک کتاب خوب دارم.'):
                selected, marks = render(voice, (EciIndex(1), text, EciIndex(2)))
                direct, _ = render(persian, (text,))
                assert selected != direct and len(selected) == len(direct), text
                assert marks == [(1, 0), (2, len(selected))], text
            mixed, marks = render(voice, (EciIndex(3), 'NVDA ', EciIndex(4), 'کتاب', EciIndex(5)))
            assert marks[0] == (3, 0) and marks[-1] == (5, len(mixed)) and 0 < marks[1][1] < len(mixed)
            mixed, marks = render(voice, (EciIndex(3), 'NVDA ', EciIndex(4), 'را', EciIndex(5)))
            assert marks[0] == (3, 0) and marks[-1] == (5, len(mixed)) and 0 < marks[1][1] < len(mixed)
            voice.select_voice(US_ENGLISH.id, '1')
            mixed, _ = render(voice, ('NVDA ', EciLanguage(PERSIAN_PROBE.id, '1'), 'کتاب'))
            assert mixed and getattr(voice, '_persian_korean_contour', None) is None
            voice.select_voice(PERSIAN_PROBE.id, '1')
            voice.set_parameter(NATIVE_PITCH, 65)
            voice.set_parameter(NATIVE_INFLECTION, 30)
            baseline, _ = render(voice, ('کتاب خانه ایران.',))
            voice.set_parameter(NATIVE_PITCH, 72)
            pitched, _ = render(voice, ('کتاب خانه ایران.',))
            voice.set_parameter(NATIVE_PITCH, 65)
            voice.set_parameter(NATIVE_INFLECTION, 45)
            inflected, _ = render(voice, ('کتاب خانه ایران.',))
            assert baseline != pitched and baseline != inflected
    finally:
        voice.close()
    print('Passed Persian US contour, language switches, indexes and pitch controls.')


if __name__ == '__main__':
    main()
