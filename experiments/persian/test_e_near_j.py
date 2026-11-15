"""Verify the selected Sefr-based E target next to Finnish J."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import tempfile

import numpy as np

from listening import render
from shared_sentence_contour import FRAME_TRACE, OVERRIDE, TRACE, capture
from test_selected_probe import GENDER, KEYS, NATIVE_SPEED, SAMPLE_RATE, engine, set_confirmed_persian_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--previous', type=Path, required=True)
    args = parser.parse_args()
    fa = engine(args.probe, 0x90000)
    old = engine(args.previous, 0x90000)
    en = engine(args.probe, 0x10000)
    de = engine(args.probe, 0x40000)
    checks = 0
    try:
        with tempfile.TemporaryDirectory(prefix='reloq-fa-e-check-') as folder:
            d = Path(folder)
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for e in (fa, old, en, de):
                        e.set_parameter(SAMPLE_RATE, rate)
                        e.set_parameter(GENDER, gender)
                    for text in ('`[.1jek]', '`[.1je]', '`[.1ej]'):
                        # Compare the E change to the earlier library with
                        # its original automatic onset behavior held fixed.
                        set_confirmed_persian_probe(e_mode=0, no_auto_glottal=0)
                        before = render(fa, text)
                        assert before == render(old, text), ('disabled E altered', rate, gender, text)
                        set_confirmed_persian_probe(no_auto_glottal=0)
                        selected = render(fa, text)
                        assert selected != before and len(selected) == len(before), ('E correction absent', rate, gender, text)
                        # The selected E must also work with the newly
                        # preferred no-automatic-glottal setting enabled.
                        set_confirmed_persian_probe(e_mode=0)
                        current_before = render(fa, text)
                        set_confirmed_persian_probe()
                        current = render(fa, text)
                        assert current != current_before and len(current) == len(current_before), (
                            'E with no automatic onset', rate, gender, text)
                        checks += 3
                    for text in ('`[.1sefr]', '`[.1pe]', "`[.1s'e:']", '`[.1jad]',
                                 '`[.1ji]', '`[.1sa.0wAt]', '`[.1ka.0RAz]', '`[.1sA.0lAm]'):
                        set_confirmed_persian_probe(e_mode=0)
                        before = render(fa, text)
                        set_confirmed_persian_probe()
                        assert render(fa, text) == before, ('unrelated phoneme changed', rate, gender, text)
                        checks += 1
                    for e, text in ((en, 'yes and yet'), (de, 'Jetzt geht es.')):
                        set_confirmed_persian_probe(e_mode=0)
                        before = render(e, text)
                        set_confirmed_persian_probe()
                        assert render(e, text) == before, ('other voice changed', rate, gender, text)
                        checks += 1
                    set_confirmed_persian_probe()
                    item = capture(fa, ['`[.1jek]'], d / 'f0', frames=d / 'frames')
                    c = item.frames['controls']
                    # In the voiced vowel, the three target resonances reach
                    # the measured centre of the accepted Sefr E profile.
                    expected = (495, 2268, 3004) if gender else (450, 1875, 2525)
                    matches = ((c[:, 2] > 20) & (c[:, 9] == expected[0]) &
                               (c[:, 13] == expected[1]) & (c[:, 15] == expected[2]))
                    assert np.count_nonzero(matches) >= 5, ('E target missing', rate, gender)
                    checks += 1
            for speed in (40, 98, 200):
                fa.set_parameter(SAMPLE_RATE, 3)
                fa.set_parameter(NATIVE_SPEED, speed)
                set_confirmed_persian_probe(e_mode=0)
                before = render(fa, '`[.1jek] `[.1sefr]')
                set_confirmed_persian_probe()
                after = render(fa, '`[.1jek] `[.1sefr]')
                assert before != after and len(before) == len(after), ('E or timing at speed', speed)
                checks += 1
    finally:
        for e in (fa, old, en, de):
            e.close()
        for key in (*KEYS, TRACE, OVERRIDE, FRAME_TRACE, 'RELOQ_FA_TRACE_E'):
            os.environ.pop(key, None)
    print(f'Passed {checks} E target, neighbouring-J, timing and language-isolation checks.')


if __name__ == '__main__':
    main()
